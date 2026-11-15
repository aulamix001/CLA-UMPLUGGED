#include "DspEngine.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace unplugged
{
namespace { constexpr double pi = 3.14159265358979323846;
float finite(float x, float fallback = 0) { return std::isfinite(x) ? x : fallback; }
}
float Engine::gain(float db) noexcept { return std::pow(10.0f, db / 20.0f); }
void Engine::Biquad::shelf(double sr, double hz, double db, bool high) noexcept
{
    const double A=std::pow(10.0, db/40), w=2*pi*std::min(hz,sr*0.45)/sr;
    const double c=std::cos(w), alpha=std::sin(w)/std::sqrt(2.0), t=2*std::sqrt(A)*alpha;
    double a0;
    if(high) {
        b0=A*((A+1)+(A-1)*c+t); b1=-2*A*((A-1)+(A+1)*c); b2=A*((A+1)+(A-1)*c-t);
        a0=(A+1)-(A-1)*c+t; a1=2*((A-1)-(A+1)*c); a2=(A+1)-(A-1)*c-t;
    } else {
        b0=A*((A+1)-(A-1)*c+t); b1=2*A*((A-1)-(A+1)*c); b2=A*((A+1)-(A-1)*c-t);
        a0=(A+1)+(A-1)*c+t; a1=-2*((A-1)+(A+1)*c); a2=(A+1)+(A-1)*c-t;
    }
    b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0;
}
float Engine::Biquad::tick(float x, int ch) noexcept
{
    const double y=b0*x+z1[ch]; z1[ch]=b1*x-a1*y+z2[ch]; z2[ch]=b2*x-a2*y;
    return static_cast<float>(y);
}
void Engine::Delay::prepare(std::size_t size) { data.assign(size,0); write=0; }
void Engine::Delay::clear() noexcept { std::fill(data.begin(),data.end(),0); write=0; }
float Engine::Delay::read(double samples) const noexcept
{
    samples=std::clamp(samples,1.0,static_cast<double>(data.size()-2));
    double position=static_cast<double>(write)-samples;
    if(position<0) position+=static_cast<double>(data.size());
    const double base=std::floor(position);
    // Adding the ring size to a tiny negative value can round to exactly size.
    const auto i=static_cast<std::size_t>(base)%data.size();
    const auto j=(i+1)%data.size();
    const auto fraction=static_cast<float>(position-base);
    return data[i]+fraction*(data[j]-data[i]);
}
void Engine::Delay::push(float value) noexcept { data[write]=value; write=(write+1)%data.size(); }
void Engine::Reverb::prepare(double sampleRate)
{
    sr=sampleRate;
    for(auto& line:lines) line.prepare(static_cast<std::size_t>(sr*0.16)+8);
    pre.prepare(static_cast<std::size_t>(sr*0.17)+8); clear();
}
void Engine::Reverb::clear() noexcept
{
    for(auto& line:lines) line.clear();
    pre.clear(); damp={};
}
std::array<float,2> Engine::Reverb::tick(float x,int colour,int variant,float predelay) noexcept
{
    pre.push(x);
    if(predelay>0) x=pre.read(predelay*0.001*sr);
    // Eight mutually prime combs, alternating stereo sums and damping.
    // This is an independent Schroeder-style approximation, not Waves' algorithm.
    constexpr std::array<double,8> times { .0297,.0371,.0411,.0437,.0307,.0323,.0397,.0451 };
    const float scale=0.7f+0.35f*static_cast<float>(colour)+0.22f*static_cast<float>(variant);
    const float rt60=(0.5f+0.7f*static_cast<float>(colour))*(variant==1?1.35f:1.0f);
    std::array<float,2> result {};
    for(std::size_t i=0;i<lines.size();++i) {
        const double time=times[i]*scale;
        const float delayed=lines[i].read(time*sr);
        const float feedback=std::pow(0.001f,static_cast<float>(time)/rt60);
        damp[i]=0.58f*damp[i]+0.42f*delayed;
        lines[i].push(0.23f*x+feedback*damp[i]);
        result[i/4]+=delayed*0.35f;
    }
    return result;
}
void Engine::prepare(double sampleRate)
{
    sr=std::clamp(std::isfinite(sampleRate)?sampleRate:48000.0,8000.0,384000.0);
    for(auto& r:reverbs) r.prepare(sr);
    for(auto& d:echo) d.prepare(static_cast<std::size_t>(sr*4.1)+8);
    const float smoothing=static_cast<float>(std::exp(-1.0/(sr*0.02)));
    for(auto* s:{ &inGain,&outGain,&directGain,&balance,&echoTime }) s->coefficient=smoothing;
    for(auto& s:wet) s.coefficient=smoothing;
    setSettings(settings);
    reset();
}
void Engine::reset() noexcept
{
    bass.clear(); treble.clear(); envelope=0;
    for(auto& r:reverbs) r.clear();
    for(auto& d:echo) d.clear();
    for(auto* s:{ &inGain,&outGain,&directGain,&balance,&echoTime }) s->value=s->target;
    for(auto& s:wet) s.value=s.target;
    inputPeak={}; outputPeak={};
}
void Engine::setSettings(const Settings& next) noexcept
{
    settings=next;
    settings.inputDb=std::clamp(finite(next.inputDb),-10.0f,10.0f);
    settings.outputDb=std::clamp(finite(next.outputDb),-10.0f,10.0f);
    settings.balance=std::clamp(finite(next.balance),-10.0f,10.0f);
    settings.bpm=std::clamp(std::isfinite(next.bpm)?next.bpm:120.0,20.0,400.0);
    for(int i=0;i<6;++i) {
        settings.colour[i]=std::clamp(next.colour[i],0,3);
        settings.amount[i]=std::clamp(finite(next.amount[i]),-10.0f,10.0f);
    }
    for(int i=0;i<2;++i) settings.preDelayMs[i]=std::clamp(finite(next.preDelayMs[i],16),0.0f,160.0f);
    constexpr double lows[] { 80,80,180,350 }, highs[] { 2500,2500,6500,11000 };
    bass.shelf(sr,lows[settings.colour[0]],settings.colour[0]?settings.amount[0]:0,false);
    treble.shelf(sr,highs[settings.colour[1]],settings.colour[1]?settings.amount[1]:0,true);
    attack=static_cast<float>(std::exp(-1.0/(sr*(settings.colour[2]==1?.020:.004))));
    release=static_cast<float>(std::exp(-1.0/(sr*(settings.colour[2]==3?.20:.10))));
    inGain.target=gain(settings.inputDb); outGain.target=gain(settings.outputDb);
    directGain.target=settings.direct?1.0f:0.0f; balance.target=settings.balance/10.0f;
    for(int i=0;i<3;++i) {
        const float level=(settings.amount[i+3]+10.0f)/20.0f;
        wet[i].target=settings.colour[i+3]?level*level*0.75f:0;
    }
    double seconds=0.12;
    if(settings.colour[5]==2) seconds=30.0/settings.bpm;
    if(settings.colour[5]==3) seconds=60.0/settings.bpm;
    echoTime.target=static_cast<float>(seconds*sr);
}
void Engine::process(float* left,float* right,int samples) noexcept
{
    if(samples<=0 || left==nullptr || right==nullptr || echo[0].data.empty()) return;
    inputPeak={}; outputPeak={};
    for(int i=0;i<samples;++i) {
        float l=finite(left[i]), r=finite(right[i]);
        const float input=inGain.next(), output=outGain.next();
        if(settings.bypass) {
            inputPeak[0]=std::max(inputPeak[0],std::abs(l)); inputPeak[1]=std::max(inputPeak[1],std::abs(r));
            outputPeak=inputPeak; left[i]=l; right[i]=r; continue;
        }
        const float pan=balance.next();
        l*=input*std::min(1.0f,1.0f-pan); r*=input*std::min(1.0f,1.0f+pan);
        inputPeak[0]=std::max(inputPeak[0],std::abs(l)); inputPeak[1]=std::max(inputPeak[1],std::abs(r));
        l=treble.tick(bass.tick(l,0),0); r=treble.tick(bass.tick(r,1),1);
        const float detector=std::max(std::abs(l),std::abs(r));
        const float coeff=detector>envelope?attack:release;
        envelope=coeff*envelope+(1-coeff)*detector;
        if(settings.colour[2]) {
            const float drive=(settings.amount[2]+10.0f)/20.0f;
            const float threshold=-8.0f-30.0f*drive;
            const float db=20*std::log10(std::max(envelope,1.0e-9f));
            const float ratio=1.5f+static_cast<float>(settings.colour[2])*1.5f;
            const float reduction=std::max(0.0f,db-threshold)*(1-1/ratio);
            const float makeup=drive*5.0f;
            const float cg=gain(makeup-reduction); l*=cg; r*=cg;
        }
        const float mono=(l+r)*0.5f;
        std::array<float,2> fx {};
        for(int j=0;j<2;++j) {
            const auto reverb=reverbs[j].tick(settings.colour[j+3]?mono:0,
                std::max(1,settings.colour[j+3]),j,settings.preDelayOn[j]?settings.preDelayMs[j]:0);
            const float mix=wet[j].next();
            fx[0]+=mix*reverb[0]; fx[1]+=mix*reverb[1];
        }
        const float dt=echoTime.next();
        const float dl=echo[0].read(dt), dr=echo[1].read(dt);
        const float echoFeed=settings.colour[5]?1.0f:0.0f;
        // Equal tempo on both channels; cross-feedback creates stereo repeats.
        echo[0].push(echoFeed*l+0.28f*dr); echo[1].push(echoFeed*r+0.28f*dl);
        const float dm=wet[2].next(), dry=directGain.next();
        l=(l*dry+fx[0]+dl*dm)*output;
        r=(r*dry+fx[1]+dr*dm)*output;
        left[i]=finite(l); right[i]=finite(r);
        outputPeak[0]=std::max(outputPeak[0],std::abs(left[i]));
        outputPeak[1]=std::max(outputPeak[1],std::abs(right[i]));
    }
}
}
