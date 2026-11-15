#include "PluginEditor.h"
#include <BinaryData.h>
#include <cmath>

namespace {
constexpr int artWidth=1604,artHeight=980,footerHeight=110;
juce::Colour colourFor(int i) {
    const juce::Colour colours[] {juce::Colour(0xff55564a),juce::Colour(0xff9cdd42),
        juce::Colour(0xff43c7f3),juce::Colour(0xffee5145)};
    return colours[juce::jlimit(0,3,i)];
}
}
void UnpluggedEditor::Skin::drawLinearSlider(juce::Graphics& g,int x,int y,int w,int h,
    float sliderPos,float,float,juce::Slider::SliderStyle style,juce::Slider& s)
{
    if(style!=juce::Slider::LinearVertical) {
        LookAndFeel_V4::drawLinearSlider(g,x,y,w,h,sliderPos,0,0,style,s); return;
    }
    const auto r=s.getLocalBounds().toFloat();
    // Compositing at runtime only: the embedded PNG is never rewritten.
    g.drawImage(art,0,0,static_cast<int>(r.getWidth()),static_cast<int>(r.getHeight()),
        215,430,40,410);
    const float cx=r.getCentreX();
    if(s.getComponentID()=="reverb1" || s.getComponentID()=="reverb2") {
        g.setColour(juce::Colour(0xffa68d4d)); g.fillRect(cx-13,0.0f,26.0f,r.getHeight());
    }
    g.setColour(juce::Colours::black); g.fillRoundedRectangle(cx-5,3,10,r.getHeight()-6,4);
    g.setColour(juce::Colour(0xff505052)); g.drawLine(cx+6,5,cx+6,r.getHeight()-5,1);
    // Silver handle sampled directly from the unchanged source artwork.
    g.drawImage(art,static_cast<int>(cx)-43,static_cast<int>(sliderPos)-63,86,126,
        446,508,86,126);
}
void UnpluggedEditor::Skin::drawButtonBackground(juce::Graphics& g,juce::Button& b,
    const juce::Colour& background,bool over,bool down)
{
    if(!static_cast<bool>(b.getProperties().getWithDefault("artworkButton",false))) {
        LookAndFeel_V4::drawButtonBackground(g,b,background,over,down); return;
    }
    auto r=b.getLocalBounds().toFloat().reduced(1);
    g.setColour(juce::Colours::black.withAlpha(.7f)); g.fillRoundedRectangle(r.translated(2,3),2);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffb7b9ac),r.getTopLeft(),
        juce::Colour(0xff333632),r.getBottomRight(),false));
    g.fillRect(r); r.reduce(5,5);
    const auto c=background.brighter(over?.3f:0.0f).darker(down?.25f:0.0f);
    g.setGradientFill(juce::ColourGradient(c.brighter(.5f),r.getTopLeft(),c.darker(.4f),r.getBottomRight(),false));
    g.fillRect(r); g.setColour(c.brighter(.7f)); g.drawHorizontalLine(static_cast<int>(r.getY()),r.getX(),r.getRight());
}
void UnpluggedEditor::Skin::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down)
{
    if(!static_cast<bool>(b.getProperties().getWithDefault("artworkButton",false))) {
        LookAndFeel_V4::drawButtonText(g,b,over,down); return;
    }
    g.setColour(juce::Colour(0xff25271a)); g.setFont(juce::Font(juce::FontOptions(21.0f,juce::Font::bold)));
    g.drawText(b.getButtonText(),b.getLocalBounds(),juce::Justification::centred);
}
void UnpluggedEditor::Skin::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,
    float pos,float start,float end,juce::Slider&)
{
    auto r=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(w),static_cast<float>(h)).reduced(5);
    const float cx=r.getCentreX(),cy=r.getCentreY(),radius=std::min(r.getWidth(),r.getHeight())*.5f;
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffead57f),cx-radius,cy-radius,
        juce::Colour(0xff79652a),cx+radius,cy+radius,false));
    g.fillEllipse(cx-radius,cy-radius,2*radius,2*radius);
    g.setColour(juce::Colour(0xff151719)); g.fillEllipse(cx-radius+6,cy-radius+6,2*radius-12,2*radius-12);
    const float angle=start+pos*(end-start);
    g.setColour(juce::Colours::whitesmoke);
    g.drawLine(cx+std::sin(angle)*radius*.42f,cy-std::cos(angle)*radius*.42f,
        cx+std::sin(angle)*radius*.84f,cy-std::cos(angle)*radius*.84f,4);
}
UnpluggedEditor::UnpluggedEditor(UnpluggedProcessor& p)
    : AudioProcessorEditor(p),processor(p),
      artwork(juce::ImageFileFormat::loadFrom(BinaryData::claunpluggedoriginal_png,BinaryData::claunpluggedoriginal_pngSize)),
      skin(artwork)
{
    setLookAndFeel(&skin); addAndMakeVisible(controls); controls.setInterceptsMouseClicks(false,true);
    const std::array<juce::String,8> ids {"input","bass","treble","compress","reverb1","reverb2","delay","output"};
    const std::array<int,8> xs {170,488,618,748,878,1008,1138,1454};
    for(std::size_t i=0;i<faders.size();++i) {
        auto& s=faders[i]; controls.addAndMakeVisible(s); s.setComponentID(ids[i]);
        s.setName(ids[i]); s.setSliderStyle(juce::Slider::LinearVertical);
        s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        s.setBounds(xs[i]-48,430,96,410); s.setDoubleClickReturnValue(true,0);
        s.setPopupDisplayEnabled(true,true,this); s.setTooltip(ids[i]+" | double-click: 0 | Shift: fine adjustment");
        attachments.push_back(std::make_unique<Attachment>(p.state,ids[i],s));
    }
    for(int i=0;i<2;++i) {
        auto& s=predelay[i]; controls.addAndMakeVisible(s);
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        s.setBounds(120+i*140,207,104,94); s.setName("Reverb "+juce::String(i+1)+" predelay");
        s.setDoubleClickReturnValue(true,16); s.setTextValueSuffix(" ms"); s.setPopupDisplayEnabled(true,true,this);
        s.setTooltip(s.getName()+" | 0-160 ms");
        attachments.push_back(std::make_unique<Attachment>(p.state,"predelay"+juce::String(i+1),s));
        auto& b=predelayButtons[i]; controls.addAndMakeVisible(b); b.setBounds(82+i*140,198,34,32);
        b.getProperties().set("artworkButton",true);
        b.setClickingTogglesState(true); b.setTooltip("Reverb "+juce::String(i+1)+" predelay on/off");
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"predelay"+juce::String(i+1)+"On",b));
    }
    for(int i=0;i<6;++i) {
        auto& b=colourButtons[i]; controls.addAndMakeVisible(b); b.setButtonText(juce::String::charToString(0x2191));
        b.getProperties().set("artworkButton",true);
        b.setBounds(449+i*130,315,78,46); b.setTooltip(juce::String(UnpluggedProcessor::stages[i])+" colour: OFF / "+UnpluggedProcessor::colourNames[i].joinIntoString(" / "));
        b.onClick=[this,i] {
            auto* parameter=processor.state.getParameter(juce::String(UnpluggedProcessor::stages[i])+"Colour");
            const int current=static_cast<int>(parameter->convertFrom0to1(parameter->getValue()));
            parameter->beginChangeGesture(); parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>((current+1)%4))); parameter->endChangeGesture(); repaint();
        };
    }
    controls.addAndMakeVisible(balance); balance.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    balance.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0); balance.setBounds(281,816,51,51);
    balance.setDoubleClickReturnValue(true,0); balance.setTooltip("Stereo balance | double-click: centre");
    attachments.push_back(std::make_unique<Attachment>(p.state,"balance",balance));
    controls.addAndMakeVisible(direct); direct.setBounds(1460,882,84,32); direct.setButtonText("DIRECT"); direct.setClickingTogglesState(true);
    direct.getProperties().set("artworkButton",true);
    direct.setTooltip("Direct signal on/off. Off = effects-only return.");
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"direct",direct));
    for(auto* c:std::array<juce::Component*,6>{&categories,&presets,&previous,&next,&bypass,&heading}) controls.addAndMakeVisible(c);
    heading.setText("FACTORY PRESETS  |  Unplugged Study",juce::dontSendNotification);
    heading.setBounds(44,988,1150,30); heading.setColour(juce::Label::textColourId,juce::Colour(0xffd8c080));
    categories.setBounds(44,1028,320,40); presets.setBounds(380,1028,805,40);
    previous.setBounds(1200,1028,60,40); previous.setButtonText("<");
    next.setBounds(1270,1028,60,40); next.setButtonText(">");
    bypass.setBounds(1360,1028,190,40); bypass.setButtonText("BYPASS"); bypass.setClickingTogglesState(true);
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass));
    juce::StringArray groups;
    for(const auto& preset:unplugged::factoryPresets()) groups.addIfNotAlreadyThere(preset.category);
    categories.addItemList(groups,1); categories.setSelectedId(1,juce::dontSendNotification);
    categories.onChange=[this]{populatePresets();};
    presets.onChange=[this]{if(presets.getSelectedId()>0) processor.loadPreset(presets.getSelectedId()-1);};
    previous.onClick=[this]{processor.loadPreset((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms()); syncPreset();};
    next.onClick=[this]{processor.loadPreset((processor.getCurrentProgram()+1)%processor.getNumPrograms()); syncPreset();};
    setResizable(true,true); getConstrainer()->setFixedAspectRatio(static_cast<double>(artWidth)/(artHeight+footerHeight));
    setResizeLimits(802,545,1604,1090); setSize(1100,748);
    syncPreset(); timerCallback(); startTimerHz(30);
}
UnpluggedEditor::~UnpluggedEditor()
{
    stopTimer(); attachments.clear(); buttonAttachments.clear(); setLookAndFeel(nullptr);
}
void UnpluggedEditor::populatePresets()
{
    presets.clear(juce::dontSendNotification);
    const auto group=categories.getText(); const auto& bank=unplugged::factoryPresets();
    for(std::size_t i=0;i<bank.size();++i) if(juce::String(bank[i].category)==group)
        presets.addItem(bank[i].name,static_cast<int>(i)+1);
    presets.setSelectedId(processor.getCurrentProgram()+1,juce::dontSendNotification);
}
void UnpluggedEditor::syncPreset()
{
    const int index=processor.getCurrentProgram();
    if(index==shownPreset) return;
    shownPreset=index; const auto& entry=unplugged::factoryPresets()[static_cast<std::size_t>(index)];
    categories.setText(entry.category,juce::dontSendNotification); populatePresets();
    presets.setSelectedId(index+1,juce::dontSendNotification);
}
void UnpluggedEditor::timerCallback()
{
    for(int ch=0;ch<2;++ch) {
        inDisplay[ch]=std::max(processor.inputMeter[ch].exchange(0),inDisplay[ch]*.86f);
        outDisplay[ch]=std::max(processor.outputMeter[ch].exchange(0),outDisplay[ch]*.86f);
    }
    for(int i=0;i<6;++i) {
        const int mode=static_cast<int>(processor.state.getRawParameterValue(juce::String(UnpluggedProcessor::stages[i])+"Colour")->load());
        colourButtons[i].setColour(juce::TextButton::buttonColourId,colourFor(mode).darker(.3f));
    }
    for(auto* b:{&predelayButtons[0],&predelayButtons[1],&direct,&bypass}) {
        b->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xffbab444));
        b->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff3b3c38));
    }
    syncPreset(); repaint();
    balance.setEnabled(processor.getTotalNumInputChannels()==2);
}
void UnpluggedEditor::drawMeter(juce::Graphics& g,float x,const std::array<float,2>& peak)
{
    for(int ch=0;ch<2;++ch) {
        g.setColour(juce::Colour(0xff111b13));
        g.fillRect(x+static_cast<float>(ch)*48,444.0f,18.0f,360.0f);
        const float db=juce::Decibels::gainToDecibels(peak[ch],-60.0f);
        const int illuminated=juce::jlimit(0,27,static_cast<int>(std::ceil((db+26.0f)*27.0f/26.0f)));
        for(int segment=0;segment<27;++segment) {
            const auto lit=segment<illuminated;
            auto c=segment>=25?juce::Colour(0xffe9443e):segment>=22?juce::Colour(0xffdbc63d):juce::Colour(0xff8cf51c);
            if(!lit) c=c.darker(2.5f);
            g.setColour(c); g.fillRect(x+static_cast<float>(ch)*48,792.0f-static_cast<float>(segment)*13,18.0f,10.0f);
        }
    }
}
void UnpluggedEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff191b1d));
    g.addTransform(juce::AffineTransform::scale(static_cast<float>(getWidth())/artWidth));
    g.drawImageAt(artwork,0,0);
    g.setColour(juce::Colour(0xff1b1d1e)); g.fillRect(429,201,768,87);
    g.setFont(juce::Font(juce::FontOptions(20.0f,juce::Font::bold|juce::Font::italic)));
    for(int i=0;i<6;++i) {
        const int selected=static_cast<int>(processor.state.getRawParameterValue(juce::String(UnpluggedProcessor::stages[i])+"Colour")->load());
        for(int row=0;row<3;++row) {
            const int mode=3-row;
            g.setColour(mode==selected?colourFor(selected):juce::Colour(0xff666868));
            g.drawText(UnpluggedProcessor::colourNames[i][mode],432+i*130,204+row*27,120,26,juce::Justification::centred);
        }
    }
    drawMeter(g,274,inDisplay); drawMeter(g,1282,outDisplay);
    const float inputDb=juce::Decibels::gainToDecibels(std::max(inDisplay[0],inDisplay[1]),-90.0f);
    const auto led=inputDb>=0?juce::Colours::red:inputDb>=-6?juce::Colours::orange:
        inputDb>=-36?juce::Colours::limegreen:juce::Colour(0xff163317);
    g.setColour(led); g.fillRoundedRectangle(157,387,27,15,3);
    if(processor.clipped.load()) {
        g.setColour(juce::Colours::red); g.fillRect(1282,445,66,10);
    }
}
void UnpluggedEditor::resized()
{
    controls.setBounds(0,0,artWidth,artHeight+footerHeight);
    controls.setTransform(juce::AffineTransform::scale(static_cast<float>(getWidth())/artWidth));
}
void UnpluggedEditor::mouseDown(const juce::MouseEvent& event)
{
    const auto point=event.position*static_cast<float>(artWidth)/static_cast<float>(getWidth());
    if(juce::Rectangle<float>(1270,430,100,390).contains(point)) processor.clipped.store(false);
}
