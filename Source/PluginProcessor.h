#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "DspEngine.h"
#include "Presets.h"

class UnpluggedProcessor final : public juce::AudioProcessor
{
public:
    UnpluggedProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Unplugged Study"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 16; }
    int getNumPrograms() override { return static_cast<int>(unplugged::factoryPresets().size()); }
    int getCurrentProgram() override { return currentPreset.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;
    juce::AudioProcessorValueTreeState state;
    std::array<std::atomic<float>,2> inputMeter {}, outputMeter {};
    std::atomic<bool> clipped {false};
    void loadPreset(int);
    static juce::AudioProcessorValueTreeState::ParameterLayout layout();
    static constexpr std::array<const char*,6> stages { "bass","treble","compress","reverb1","reverb2","delay" };
    static const std::array<juce::StringArray,6> colourNames;
private:
    unplugged::Engine engine;
    std::atomic<int> currentPreset {0};
    struct StagePointers { std::atomic<float>* amount; std::atomic<float>* colour; };
    std::array<StagePointers,6> stageParameters {};
    std::atomic<float> *input=nullptr,*output=nullptr,*balance=nullptr,*direct=nullptr,*bypass=nullptr;
    std::array<std::atomic<float>*,2> predelay {}, predelayOn {};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnpluggedProcessor)
};
