#pragma once
#include "PluginProcessor.h"

class UnpluggedEditor final : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit UnpluggedEditor(UnpluggedProcessor&);
    ~UnpluggedEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent&) override;
private:
    using Attachment=juce::AudioProcessorValueTreeState::SliderAttachment;
    class Skin final : public juce::LookAndFeel_V4
    {
    public:
        explicit Skin(juce::Image image):art(std::move(image)) {}
        int getSliderThumbRadius(juce::Slider& s) override { return s.isVertical()?55:10; }
        void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,
            juce::Slider::SliderStyle,juce::Slider&) override;
        void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
        void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
        void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    private:
        juce::Image art;
    };
    UnpluggedProcessor& processor;
    juce::Image artwork;
    Skin skin;
    juce::Component controls;
    std::array<juce::Slider,8> faders;
    std::array<juce::Slider,2> predelay;
    juce::Slider balance;
    std::vector<std::unique_ptr<Attachment>> attachments;
    std::array<juce::TextButton,6> colourButtons;
    std::array<juce::TextButton,2> predelayButtons;
    juce::TextButton direct, bypass, previous, next;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
    juce::ComboBox categories, presets;
    juce::Label heading;
    juce::TooltipWindow tooltips {this,500};
    std::array<float,2> inDisplay {}, outDisplay {};
    int shownPreset=-1;
    void timerCallback() override;
    void populatePresets();
    void syncPreset();
    void drawMeter(juce::Graphics&,float,const std::array<float,2>&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UnpluggedEditor)
};
