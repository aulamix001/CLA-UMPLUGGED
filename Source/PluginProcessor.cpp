#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::array<juce::StringArray,6> UnpluggedProcessor::colourNames {{
    {"OFF","SUB","LOWER","UPPER"}, {"OFF","BITE","TOP","ROOF"},
    {"OFF","PUSH","SPANK","WALL"}, {"OFF","ROOM","HALL","CHAMBER"},
    {"OFF","TIGHT","LARGE","CANYON"}, {"OFF","SLAP","EIGHT","QUARTER"}
}};
juce::AudioProcessorValueTreeState::ParameterLayout UnpluggedProcessor::layout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto addFloat=[&](const juce::String& id,const juce::String& name,float low,float high,float step,float value) {
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,
            juce::NormalisableRange<float>{low,high,step},value));
    };
    addFloat("input","Input sensitivity (dB)",-10,10,.1f,0);
    addFloat("output","Output (dB)",-10,10,.1f,0);
    addFloat("balance","Balance (dB)",-6,6,.1f,0);
    for(std::size_t i=0;i<stages.size();++i) {
        const juce::String id=stages[i];
        addFloat(id,id.toUpperCase(),-10,10,.1f,0);
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id+"Colour",1},
            id+" colour",colourNames[i],1));
    }
    for(int i=1;i<=2;++i) {
        const auto id="predelay"+juce::String(i);
        juce::StringArray notes;
        for(const auto* note:unplugged::preDelayNames) notes.add(note);
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{id,1},
            "Reverb "+juce::String(i)+" predelay (note)",notes,7));
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{id+"On",1},id+" on",false));
    }
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"direct",1},"Direct",true));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"bypass",1},"Bypass",false));
    return p;
}
UnpluggedProcessor::UnpluggedProcessor()
    : AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
        .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      state(*this,nullptr,"UnpluggedState",layout())
{
    (void)unplugged::factoryPresets();
    input=state.getRawParameterValue("input"); output=state.getRawParameterValue("output");
    balance=state.getRawParameterValue("balance"); direct=state.getRawParameterValue("direct");
    bypass=state.getRawParameterValue("bypass");
    for(std::size_t i=0;i<stages.size();++i) stageParameters[i]={state.getRawParameterValue(stages[i]),
        state.getRawParameterValue(juce::String(stages[i])+"Colour")};
    for(int i=0;i<2;++i) {
        const auto id="predelay"+juce::String(i+1);
        predelay[i]=state.getRawParameterValue(id); predelayOn[i]=state.getRawParameterValue(id+"On");
    }
}
bool UnpluggedProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    // The manual specifies stereo and mono-to-stereo components.
    return l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()
        && (l.getMainInputChannelSet()==juce::AudioChannelSet::mono()
            || l.getMainInputChannelSet()==juce::AudioChannelSet::stereo());
}
void UnpluggedProcessor::prepareToPlay(double sampleRate,int)
{
    engine.prepare(sampleRate); clipped.store(false);
}
void UnpluggedProcessor::releaseResources() { engine.reset(); }
void UnpluggedProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    midi.clear();
    if(buffer.getNumChannels()<2) return;
    if(getTotalNumInputChannels()==1) buffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());
    unplugged::Settings s;
    s.inputDb=input->load(); s.outputDb=output->load();
    s.balance=getTotalNumInputChannels()==2?balance->load():0.0f;
    s.direct=direct->load()>.5f; s.bypass=bypass->load()>.5f;
    for(std::size_t i=0;i<stages.size();++i) {
        s.amount[i]=stageParameters[i].amount->load();
        s.colour[i]=static_cast<int>(stageParameters[i].colour->load());
    }
    for(int i=0;i<2;++i) { s.preDelayNote[i]=static_cast<int>(predelay[i]->load()); s.preDelayOn[i]=predelayOn[i]->load()>.5f; }
    if(auto* playhead=getPlayHead()) if(auto pos=playhead->getPosition())
        if(auto bpm=pos->getBpm()) s.bpm=*bpm;
    engine.setSettings(s);
    engine.process(buffer.getWritePointer(0),buffer.getWritePointer(1),buffer.getNumSamples());
    for(int ch=0;ch<2;++ch) {
        auto retainPeak=[](std::atomic<float>& meter,float value) {
            auto old=meter.load(std::memory_order_relaxed);
            while(old<value && !meter.compare_exchange_weak(old,value,std::memory_order_relaxed)) {}
        };
        retainPeak(inputMeter[ch],engine.inputPeak[ch]);
        retainPeak(outputMeter[ch],engine.outputPeak[ch]);
        if(engine.outputPeak[ch]>1.0f) clipped.store(true,std::memory_order_relaxed);
    }
}
void UnpluggedProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals; midi.clear();
    if(getTotalNumInputChannels()==1 && buffer.getNumChannels()>1)
        buffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());
}
juce::AudioProcessorParameter* UnpluggedProcessor::getBypassParameter() const { return state.getParameter("bypass"); }
juce::AudioProcessorEditor* UnpluggedProcessor::createEditor() { return new UnpluggedEditor(*this); }
void UnpluggedProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto snapshot=state.copyState(); snapshot.setProperty("version",2,nullptr);
    snapshot.setProperty("presetIndex",currentPreset.load(),nullptr);
    if(auto xml=snapshot.createXml()) copyXmlToBinary(*xml,destination);
}
void UnpluggedProcessor::setStateInformation(const void* data,int bytes)
{
    if(data==nullptr || bytes<=0) return;
    if(auto xml=getXmlFromBinary(data,bytes))
        if(xml->hasTagName(state.state.getType())) {
            auto restored=juce::ValueTree::fromXml(*xml);
            if(xml->getIntAttribute("version",1)<2) {
                // v1 stored milliseconds; convert to the closest musical division at 120 BPM.
                for(auto child:restored) {
                    const auto id=child.getProperty("id").toString();
                    if(id=="predelay1" || id=="predelay2")
                        child.setProperty("value",unplugged::nearestPreDelay(static_cast<float>(child.getProperty("value"))),nullptr);
                    if(id=="balance") child.setProperty("value",juce::jlimit(-6.0f,6.0f,static_cast<float>(child.getProperty("value"))),nullptr);
                }
            }
            state.replaceState(restored);
            currentPreset.store(juce::jlimit(0,getNumPrograms()-1,xml->getIntAttribute("presetIndex",0)));
        }
}
const juce::String UnpluggedProcessor::getProgramName(int index)
{
    return juce::isPositiveAndBelow(index,getNumPrograms()) ? unplugged::factoryPresets()[static_cast<std::size_t>(index)].name : juce::String{};
}
void UnpluggedProcessor::setCurrentProgram(int index) { loadPreset(index); }
void UnpluggedProcessor::loadPreset(int index)
{
    if(!juce::isPositiveAndBelow(index,getNumPrograms())) return;
    const auto& preset=unplugged::factoryPresets()[static_cast<std::size_t>(index)];
    const auto& s=preset.settings;
    auto set=[&](const juce::String& id,float value) {
        if(auto* p=state.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(value));
    };
    // The reference XML marks sensitivity and output as IgnoreOnPresetLoad.
    if(preset.category!="XML Reference") { set("input",s.inputDb); set("output",s.outputDb); }
    set("balance",s.balance);
    set("direct",s.direct?1.0f:0.0f); set("bypass",s.bypass?1.0f:0.0f);
    for(std::size_t i=0;i<stages.size();++i) { set(stages[i],s.amount[i]); set(juce::String(stages[i])+"Colour",static_cast<float>(s.colour[i])); }
    for(int i=0;i<2;++i) {
        const auto id="predelay"+juce::String(i+1);
        set(id,static_cast<float>(s.preDelayNote[i])); set(id+"On",s.preDelayOn[i]?1.0f:0.0f);
    }
    currentPreset.store(index);
    updateHostDisplay(juce::AudioProcessor::ChangeDetails().withProgramChanged(true));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new UnpluggedProcessor(); }
