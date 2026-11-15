#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <iostream>
#include <stdexcept>

static void check(bool condition,const char* message) { if(!condition) throw std::runtime_error(message); }
int main(int argc,char** argv)
{
    juce::ScopedJuceInitialiser_GUI initialise;
    try {
        UnpluggedProcessor p;
        check(p.getNumPrograms()>=100,"missing programs");
        p.loadPreset(2);
        juce::MemoryBlock saved; p.getStateInformation(saved);
        const float expected=p.state.getRawParameterValue("treble")->load();
        p.loadPreset(5); p.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        check(p.getCurrentProgram()==2,"preset recall");
        check(p.state.getRawParameterValue("treble")->load()==expected,"parameter recall");
        std::cout<<"PASS state and factory program recall\n";
        for(int i=0;i<p.getNumPrograms();++i) {
            p.loadPreset(i); const auto& s=unplugged::factoryPresets()[static_cast<std::size_t>(i)].settings;
            for(std::size_t j=0;j<6;++j) {
                check(std::abs(p.state.getRawParameterValue(UnpluggedProcessor::stages[j])->load()-s.amount[j])<.001f,"preset amount mapping");
                check(static_cast<int>(p.state.getRawParameterValue(juce::String(UnpluggedProcessor::stages[j])+"Colour")->load())==s.colour[j],"preset colour mapping");
            }
            const auto matches=[&](const juce::String& id,float value) {
                return std::abs(p.state.getRawParameterValue(id)->load()-value)<.001f;
            };
            check(matches("input",s.inputDb) && matches("output",s.outputDb)
                && matches("balance",s.balance),"preset gain mapping");
            check(matches("direct",s.direct?1.0f:0.0f) && matches("bypass",s.bypass?1.0f:0.0f),"preset bypass mapping");
            for(int j=0;j<2;++j) {
                const auto id="predelay"+juce::String(j+1);
                check(matches(id,s.preDelayMs[j]) && matches(id+"On",s.preDelayOn[j]?1.0f:0.0f),"preset predelay mapping");
            }
        }
        std::cout<<"PASS automatic mapping of all factory presets\n";
        auto buses=p.getBusesLayout(); buses.inputBuses.set(0,juce::AudioChannelSet::mono());
        check(p.setBusesLayout(buses),"mono-stereo layout rejected");
        p.prepareToPlay(48000,128); p.loadPreset(0);
        p.state.getParameter("bypass")->setValueNotifyingHost(1);
        juce::AudioBuffer<float> audio(2,128); audio.clear(); audio.setSample(0,0,.5f);
        juce::MidiBuffer midi; p.processBlock(audio,midi);
        check(audio.getSample(0,0)==.5f && audio.getSample(1,0)==.5f,"mono-stereo bypass mismatch");
        p.releaseResources();
        std::cout<<"PASS mono-to-stereo audio and bypass\n";
        buses.inputBuses.set(0,juce::AudioChannelSet::stereo());
        check(p.setBusesLayout(buses),"stereo layout rejected"); p.loadPreset(0);
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        check(editor && editor->getWidth()>0,"editor construction");
        auto screenshot=editor->createComponentSnapshot(editor->getLocalBounds());
        check(screenshot.isValid(),"editor rendering");
        const auto file=juce::File::getCurrentWorkingDirectory().getChildFile("editor-preview.png");
        auto stream=file.createOutputStream(); check(stream!=nullptr,"open preview output");
        stream->setPosition(0); check(stream->truncate().wasOk(),"truncate preview output");
        juce::PNGImageFormat png; check(png.writeImageToStream(screenshot,*stream),"write preview");
        stream.reset();
        std::cout<<"PASS editor construction and rendering\n";
        auto* canvas=editor->getChildComponent(0);
        check(canvas!=nullptr,"editor control container missing");
        auto* bass=dynamic_cast<juce::Slider*>(canvas->findChildWithID("bass"));
        check(bass!=nullptr,"bass control missing");
        bass->setValue(4.2,juce::sendNotificationSync);
        check(std::abs(p.state.getRawParameterValue("bass")->load()-4.2f)<.001f,"GUI-to-parameter mapping failed");
        auto* bassParameter=p.state.getParameter("bass");
        bassParameter->setValueNotifyingHost(bassParameter->convertTo0to1(-3.0f));
        check(std::abs(bass->getValue()+3.0)<.001,"automation-to-GUI mapping failed");
        p.loadPreset(0);
        std::cout<<"PASS bidirectional GUI and host parameter mapping\n";
        if(argc>1) {
            juce::VST3PluginFormat format;
            juce::OwnedArray<juce::PluginDescription> descriptions;
            const auto path=juce::File(argv[1]).getFullPathName();
            format.findAllTypesForFile(descriptions,path);
            check(descriptions.size()==1,"VST3 scan did not find exactly one plugin");
            juce::String error;
            auto instance=format.createInstanceFromDescription(*descriptions[0],48000,128,error);
            if(!instance) std::cerr<<error<<'\n';
            check(instance!=nullptr,"VST3 module failed to instantiate");
            instance->prepareToPlay(48000,128);
            check(instance->getParameters().size()>=21,"VST3 automation parameters missing");
            check(instance->getNumPrograms()>=100,"VST3 factory program list missing");
            instance->setCurrentProgram(3);
            juce::AudioBuffer<float> testAudio(2,128); testAudio.clear();
            testAudio.setSample(0,0,.25f); testAudio.setSample(1,0,.25f);
            instance->processBlock(testAudio,midi);
            float maximum=0;
            for(int ch=0;ch<2;++ch) for(int i=0;i<128;++i) {
                const auto sample=testAudio.getSample(ch,i);
                check(std::isfinite(sample),"VST3 emitted invalid audio"); maximum=std::max(maximum,std::abs(sample));
            }
            check(maximum>0,"VST3 emitted no audio");
            juce::MemoryBlock vstState; instance->getStateInformation(vstState);
            check(vstState.getSize()>0,"VST3 state missing");
            instance->setStateInformation(vstState.getData(),static_cast<int>(vstState.getSize()));
            instance->releaseResources();
            std::cout<<"PASS compiled VST3 scan, instantiate, automation, programs, audio and state\n";
        }
        return 0;
    } catch(const std::exception& e) { std::cerr<<"FAIL "<<e.what()<<'\n'; return 1; }
}
