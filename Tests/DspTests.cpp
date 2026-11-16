#include "DspEngine.h"
#include "Presets.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <new>
#include <random>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace {
bool forbidAllocations=false;
int passed=0;
void check(bool condition,const char* message) { if(!condition) throw std::runtime_error(message); }
template<class Fn> void test(const char* name,Fn fn) { fn(); ++passed; std::cout<<"PASS "<<name<<'\n'; }
float energy(const std::vector<float>& x) { double sum=0; for(float v:x) sum+=v*v; return static_cast<float>(sum/x.size()); }
unplugged::Settings drySettings() { unplugged::Settings s; s.colour.fill(0); return s; }
}
void* operator new(std::size_t size) {
    if(forbidAllocations) throw std::bad_alloc();
    if(void* ptr=std::malloc(std::max<std::size_t>(size,1))) return ptr;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* p) noexcept { std::free(p); }
void operator delete[](void* p) noexcept { std::free(p); }
void operator delete(void* p,std::size_t) noexcept { std::free(p); }
void operator delete[](void* p,std::size_t) noexcept { std::free(p); }

int main()
{
    try {
        test("dry unity and independent stereo",[]{
            unplugged::Engine e; e.setSettings(drySettings()); e.prepare(48000);
            std::vector<float> l(1024,.25f),r(1024,-.4f);
            e.process(l.data(),r.data(),1024);
            check(std::abs(l.back()-.25f)<1e-6f && std::abs(r.back()+.4f)<1e-6f,"dry stereo mismatch");
        });
        test("hard bypass ignores gain/effects",[]{
            unplugged::Engine e; unplugged::Settings s; s.bypass=true; s.inputDb=10; s.outputDb=10;
            e.setSettings(s); e.prepare(44100); std::vector<float> l(128,.37f),r(128,-.13f);
            e.process(l.data(),r.data(),128); check(l.back()==.37f && r.back()==-.13f,"bypass mismatch");
        });
        test("direct off removes dry signal",[]{
            unplugged::Engine e; auto s=drySettings(); s.direct=false;
            e.setSettings(s); e.prepare(48000); std::vector<float> l(4096,.2f),r=l;
            e.process(l.data(),r.data(),4096); check(energy(l)==0 && energy(r)==0,"direct off leaked dry signal");
        });
        test("input/output calibrated dB gain",[]{
            unplugged::Engine e; auto s=drySettings(); s.inputDb=6; s.outputDb=-3;
            e.setSettings(s); e.prepare(48000); std::vector<float> l(64,.1f),r=l;
            e.process(l.data(),r.data(),64); check(std::abs(l.back()-.1f*std::pow(10.0f,3.0f/20))<1e-6,"gain calibration");
        });
        test("tempo delay quarter/eighth/slap timing",[]{
            for(double sr:{44100.0,48000.0,96000.0}) for(int colour:{1,2,3}) {
                unplugged::Engine e; auto s=drySettings(); s.direct=false; s.colour[5]=colour; s.amount[5]=10; s.bpm=120;
                e.setSettings(s); e.prepare(sr);
                const int target=static_cast<int>(std::round(sr*(colour==1?.12:colour==2?.25:.5)));
                std::vector<float> l(static_cast<std::size_t>(target+4),0),r=l; l[0]=r[0]=1;
                e.process(l.data(),r.data(),static_cast<int>(l.size()));
                const auto peak=std::max_element(l.begin(),l.end());
                check(std::abs(static_cast<int>(peak-l.begin())-target)<=1,"wrong echo time");
                check(*peak>.5f,"missing echo");
            }
        });
        test("predelay shifts reverb onset",[]{
            auto onset=[](int note,double bpm,bool enabled) {
                unplugged::Engine e; auto s=drySettings(); s.direct=false; s.colour[3]=1;
                s.amount[3]=10; s.preDelayNote[0]=note; s.preDelayOn[0]=enabled; s.bpm=bpm; e.setSettings(s); e.prepare(48000);
                std::vector<float> l(160000,0),r=l; l[0]=r[0]=1;
                e.process(l.data(),r.data(),static_cast<int>(l.size()));
                for(std::size_t i=0;i<l.size();++i) if(std::abs(l[i])>1e-7f || std::abs(r[i])>1e-7f) return static_cast<int>(i);
                return -1;
            };
            // Independent timing anchors: sixteenth = 125 ms and quarter = 500 ms at 120 BPM.
            check(std::abs(unplugged::preDelayMilliseconds(7,120)-125)<.001f,"sixteenth-note default timing");
            check(std::abs(unplugged::preDelayMilliseconds(13,120)-500)<.001f,"quarter-note timing");
            check(std::abs(unplugged::preDelayMilliseconds(0,120)-20.833333f)<.001f,"64th triplet timing");
            const auto a=onset(7,120,false);
            for(auto bpm:{20.0,60.0,120.0,240.0}) for(int note=0;note<14;++note) {
                const auto b=onset(note,bpm,true);
                const auto expected=std::lround(48000*unplugged::preDelayMilliseconds(note,bpm)/1000);
                check(a>0 && std::abs((b-a)-expected)<=2,"wrong tempo-synced predelay onset");
            }
        });
        test("bass and treble change frequency response",[]{
            for(int stage:{0,1}) {
                unplugged::Engine e; auto s=drySettings(); s.colour[stage]=1; s.amount[stage]=8;
                e.setSettings(s); e.prepare(48000);
                std::vector<float> l(48000),r(48000);
                for(int i=0;i<48000;++i) l[i]=r[i]=.05f*std::sin(static_cast<float>(6.283185307179586*(stage==0?35:10000)*i/48000));
                e.process(l.data(),r.data(),48000); check(energy(l)>.003f,"shelf has no effect");
            }
        });
        test("Wall compressor reduces sustained loud input",[]{
            unplugged::Engine e; auto s=drySettings(); s.colour[2]=3; s.amount[2]=10;
            e.setSettings(s); e.prepare(48000); std::vector<float> l(48000,.8f),r=l;
            e.process(l.data(),r.data(),48000); check(std::abs(l.back())<.2f,"compressor ineffective");
        });
        test("balance applies calibrated 6 dB offset",[]{
            for(float offset:{-6.0f,-3.0f,0.0f,3.0f,6.0f}) {
                unplugged::Engine e; auto s=drySettings(); s.balance=offset; e.setSettings(s); e.prepare(48000);
                std::vector<float> l(128,.1f),r=l; e.process(l.data(),r.data(),128);
                const float attenuated=.1f*std::pow(10.0f,-std::abs(offset)/20);
                check(std::abs(l.back()-(offset>0?attenuated:.1f))<1e-6f
                    && std::abs(r.back()-(offset<0?attenuated:.1f))<1e-6f,"balance incorrect");
            }
        });
        test("all factory presets finite and valid at multiple sample rates",[]{
            const auto& bank=unplugged::factoryPresets(); check(bank.size()>=100,"missing presets");
            std::unordered_set<std::string> names;
            for(const auto& p:bank) check(names.insert(p.name).second,"duplicate preset name");
            for(double sr:{44100.0,48000.0,96000.0}) {
                unplugged::Engine e; e.prepare(sr); std::vector<float> l(257),r(257);
                for(const auto& preset:bank) {
                    for(auto c:preset.settings.colour) check(c>=0 && c<=3,"invalid colour");
                    for(auto a:preset.settings.amount) check(a>=-10 && a<=10,"invalid amount");
                    e.setSettings(preset.settings); e.reset();
                    for(int block=0;block<16;++block) {
                        for(int i=0;i<257;++i) { l[i]=.1f*std::sin(static_cast<float>(i+block*257)*.07f); r[i]=l[i]*.8f; }
                        e.process(l.data(),r.data(),257);
                        for(float v:l) check(std::isfinite(v) && std::abs(v)<10,"preset unstable");
                        for(float v:r) check(std::isfinite(v) && std::abs(v)<10,"preset unstable");
                    }
                }
            }
            std::cout<<"  "<<bank.size()<<" presets x 3 sample rates\n";
        });
        test("processing and automation allocate no heap memory",[]{
            unplugged::Engine e; e.prepare(48000); unplugged::Settings s;
            std::array<float,511> l {},r {}; l[0]=r[0]=.5f;
            forbidAllocations=true;
            for(int i=0;i<100;++i) { s.amount[0]=static_cast<float>(i%20)-10; e.setSettings(s); e.process(l.data(),r.data(),511); }
            forbidAllocations=false;
        });
        test("silence reset and nonfinite parameter sanitisation",[]{
            unplugged::Engine e; unplugged::Settings s;
            s.inputDb=std::numeric_limits<float>::quiet_NaN(); s.bpm=std::numeric_limits<double>::infinity();
            s.amount.fill(std::numeric_limits<float>::infinity());
            e.setSettings(s); e.prepare(192000); std::vector<float> l(1024,0),r=l;
            e.process(l.data(),r.data(),1024); check(energy(l)==0,"silence nonzero");
            l[0]=std::numeric_limits<float>::quiet_NaN(); r[0]=std::numeric_limits<float>::infinity();
            e.process(l.data(),r.data(),1024); for(float v:l) check(std::isfinite(v),"nonfinite output");
            e.reset(); e.process(nullptr,nullptr,0);
        });
        std::cout<<passed<<" tests passed\n";
        return 0;
    } catch(const std::exception& e) { forbidAllocations=false; std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
