#include "PluginEditor.h"
#include <BinaryData.h>
#include <cmath>

namespace {
constexpr int artWidth=1604,artHeight=980,toolbarHeight=56;
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
    if(static_cast<bool>(b.getProperties().getWithDefault("toolbarButton",false))) {
        auto r=b.getLocalBounds().toFloat().reduced(.5f);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff434548),r.getTopLeft(),
            juce::Colour(0xff090a0b),r.getBottomLeft(),false));
        g.fillRect(r); g.setColour(juce::Colour(0xffb7b9b7)); g.drawRect(r,1);
        g.setColour(b.getToggleState()?juce::Colour(0xff5b572c):(down?juce::Colour(0xff292c2e):juce::Colour(0xff060707))); g.fillRect(r.reduced(3));
        if(over) {g.setColour(juce::Colours::white.withAlpha(.055f));g.fillRect(r.reduced(3));}
        return;
    }
    if(!static_cast<bool>(b.getProperties().getWithDefault("artworkButton",false))) {
        LookAndFeel_V4::drawButtonBackground(g,b,background,over,down); return;
    }
    const bool lit=static_cast<bool>(b.getProperties().getWithDefault("colourSelector",false))
        ? static_cast<bool>(b.getProperties().getWithDefault("lit",true)) : b.getToggleState();
    auto r=b.getLocalBounds().toFloat().reduced(1);
    if(down)r.translate(1,1);
    const bool small=r.getWidth()<45;
    g.setColour(juce::Colour(0xcc030403)); g.fillRect(r.translated(2,3));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0a0c0d),r.getTopLeft(),
        juce::Colour(0xff515350),r.getBottomRight(),false)); g.fillRect(r);
    r.reduce(2,2);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffc1c0ac),r.getTopLeft(),
        juce::Colour(0xff282a27),r.getBottomRight(),false)); g.fillRect(r);
    r.reduce(small?2.0f:5.0f,small?2.0f:5.0f);
    const auto top=lit?juce::Colour(0xffede99a):juce::Colour(0xffaaa99c);
    const auto bottom=lit?juce::Colour(0xff85872b):juce::Colour(0xff53554c);
    g.setGradientFill(juce::ColourGradient(top,r.getTopLeft(),bottom,r.getBottomRight(),false));g.fillRect(r);
    if(lit) {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xcce8ec70),r.getCentre(),
            juce::Colour(0x005c6319),r.getTopLeft(),true));g.fillRect(r);
    }
    g.setColour(juce::Colours::black.withAlpha(down?.32f:.13f));g.drawRect(r,1);
    g.setColour(juce::Colours::white.withAlpha(over?.6f:.4f));g.drawHorizontalLine(static_cast<int>(r.getY())+1,r.getX()+1,r.getRight()-1);
}
void UnpluggedEditor::Skin::drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down)
{
    if(static_cast<bool>(b.getProperties().getWithDefault("toolbarButton",false))) {
        g.setColour(juce::Colour(0xffe6e6e3));g.setFont(juce::Font(juce::FontOptions(23.0f)));
        auto area=b.getLocalBounds().reduced(12,0);
        g.drawText(b.getButtonText(),area,juce::Justification::centred);
        if(b.getComponentID()=="presetMenu") {
            juce::Path arrow;const float x=static_cast<float>(b.getWidth()-24),y=static_cast<float>(b.getHeight())*.5f;
            arrow.addTriangle(x-5,y-3,x+5,y-3,x,y+3);g.fillPath(arrow);
        }
        return;
    }
    if(!static_cast<bool>(b.getProperties().getWithDefault("artworkButton",false))) {
        LookAndFeel_V4::drawButtonText(g,b,over,down); return;
    }
    g.setColour(juce::Colour(0xff25271a)); g.setFont(juce::Font(juce::FontOptions(21.0f,juce::Font::bold)));
    g.drawText(b.getButtonText(),b.getLocalBounds().translated(down?1:0,down?1:0),juce::Justification::centred);
}
void UnpluggedEditor::Skin::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,
    float pos,float start,float end,juce::Slider& s)
{
    // Clear the static knob in the approved background without rewriting the PNG.
    g.drawImage(art,0,0,s.getWidth(),s.getHeight(),215,
        s.getComponentID()=="balance"?816:430,40,s.getComponentID()=="balance"?51:94);
    auto r=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(w),static_cast<float>(h)).reduced(5);
    const float cx=r.getCentreX(),cy=r.getCentreY(),radius=std::min(r.getWidth(),r.getHeight())*.5f;
    // High-resolution, alpha-cut brass body keeps its lighting fixed while the
    // crisp vector indicator follows the parameter. The approved panel is intact.
    jassert(knobBody.isValid());
    const float margin=s.getComponentID()=="balance"?0.0f:5.0f;
    const auto body=juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(w),static_cast<float>(h)).expanded(margin);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage(knobBody,body);
    const float angle=start+pos*(end-start);
    const float thickness=juce::jmax(1.8f,static_cast<float>(w)*.03f);
    g.setColour(juce::Colours::black.withAlpha(.5f));
    g.drawLine(cx+std::sin(angle)*radius*.08f+1,cy-std::cos(angle)*radius*.08f+1,
        cx+std::sin(angle)*radius*.67f+1,cy-std::cos(angle)*radius*.67f+1,thickness+.5f);
    g.setColour(juce::Colour(0xfffafaf7));
    g.drawLine(cx+std::sin(angle)*radius*.08f,cy-std::cos(angle)*radius*.08f,
        cx+std::sin(angle)*radius*.67f,cy-std::cos(angle)*radius*.67f,thickness);
}
juce::Font UnpluggedEditor::Skin::getPopupMenuFont() {return juce::Font(juce::FontOptions(21.0f));}
void UnpluggedEditor::Skin::drawPopupMenuBackground(juce::Graphics& g,int w,int h)
{
    g.setColour(juce::Colour(0xff303f45));g.fillRect(0,0,w,h);g.setColour(juce::Colour(0xff849499));g.drawRect(0,0,w,h,1);
}
void UnpluggedEditor::Skin::drawPopupMenuItem(juce::Graphics& g,const juce::Rectangle<int>& area,
    bool separator,bool active,bool highlighted,bool ticked,bool submenu,const juce::String& text,
    const juce::String&,const juce::Drawable*,const juce::Colour*)
{
    if(separator) {g.setColour(juce::Colour(0xff647278));g.fillRect(area.getX()+8,area.getCentreY(),area.getWidth()-16,1);return;}
    if(highlighted&&active) {g.setColour(juce::Colour(0xff50636c));g.fillRect(area.reduced(1,0));}
    g.setColour(juce::Colour(active?0xfff3f5f4:0xff8d989b));g.setFont(getPopupMenuFont());
    auto line=area.reduced(24,0);g.drawText(text,line,juce::Justification::centredLeft);
    if(ticked) {juce::Path mark;mark.startNewSubPath(static_cast<float>(area.getX()+7),static_cast<float>(area.getCentreY()));mark.lineTo(static_cast<float>(area.getX()+11),static_cast<float>(area.getCentreY()+4));mark.lineTo(static_cast<float>(area.getX()+18),static_cast<float>(area.getCentreY()-5));g.strokePath(mark,juce::PathStrokeType(1.5f));}
    if(submenu) {const float ax=static_cast<float>(area.getRight()-15),ay=static_cast<float>(area.getCentreY());juce::Path arrow;arrow.startNewSubPath(ax-4,ay-5);arrow.lineTo(ax+1,ay);arrow.lineTo(ax-4,ay+5);g.strokePath(arrow,juce::PathStrokeType(2));}
}
UnpluggedEditor::UnpluggedEditor(UnpluggedProcessor& p)
    : AudioProcessorEditor(p),processor(p),
      artwork(juce::ImageFileFormat::loadFrom(BinaryData::claunpluggedoriginal_png,BinaryData::claunpluggedoriginal_pngSize)),
      skin(artwork,juce::ImageFileFormat::loadFrom(BinaryData::knobbody_png,BinaryData::knobbody_pngSize))
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
        s.setBounds(118+i*140,202,94,94);s.setComponentID("predelay"+juce::String(i+1)); s.setName("Reverb "+juce::String(i+1)+" predelay");
        s.setDoubleClickReturnValue(true,7);
        s.textFromValueFunction=[](double value) { return juce::String(unplugged::preDelayNames[static_cast<std::size_t>(juce::jlimit(0,13,static_cast<int>(std::round(value))))]); }; s.setPopupDisplayEnabled(true,true,this);
        s.setTooltip(s.getName()+" | 64T to 4, synced to host BPM");
        attachments.push_back(std::make_unique<Attachment>(p.state,"predelay"+juce::String(i+1),s));
        auto& b=predelayButtons[i]; controls.addAndMakeVisible(b); b.setBounds(82+i*140,198,34,32);
        b.getProperties().set("artworkButton",true);
        b.setComponentID("predelay"+juce::String(i+1)+"On");
        b.setClickingTogglesState(true); b.setTooltip("Reverb "+juce::String(i+1)+" predelay on/off");
        buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"predelay"+juce::String(i+1)+"On",b));
    }
    for(int i=0;i<6;++i) {
        auto& b=colourButtons[i]; controls.addAndMakeVisible(b); b.setButtonText(juce::String::charToString(0x2191));
        b.getProperties().set("artworkButton",true);
        b.getProperties().set("colourSelector",true);b.setComponentID(juce::String(UnpluggedProcessor::stages[i])+"Colour");
        b.setBounds(449+i*130,315,78,46); b.setTooltip(juce::String(UnpluggedProcessor::stages[i])+" colour: OFF / "+UnpluggedProcessor::colourNames[i].joinIntoString(" / "));
        b.onClick=[this,i] {
            auto* parameter=processor.state.getParameter(juce::String(UnpluggedProcessor::stages[i])+"Colour");
            const int current=static_cast<int>(parameter->convertFrom0to1(parameter->getValue()));
            parameter->beginChangeGesture(); parameter->setValueNotifyingHost(parameter->convertTo0to1(static_cast<float>((current+1)%4))); parameter->endChangeGesture(); repaint();
        };
    }
    controls.addAndMakeVisible(balance); balance.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    balance.setComponentID("balance");
    balance.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0); balance.setBounds(281,816,51,51);
    balance.setDoubleClickReturnValue(true,0); balance.setTooltip("Stereo balance | -6 to +6 dB | double-click: centre");
    attachments.push_back(std::make_unique<Attachment>(p.state,"balance",balance));
    controls.addAndMakeVisible(direct); direct.setBounds(1460,882,84,32); direct.setButtonText("DIRECT"); direct.setClickingTogglesState(true);
    direct.getProperties().set("artworkButton",true);
    direct.setTooltip("Direct signal on/off. Off = effects-only return.");
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"direct",direct));
    addAndMakeVisible(toolbar);toolbar.setComponentID("topToolbar");
    for(auto* b:{&previous,&next,&presetMenuButton,&bypass}) {
        toolbar.addAndMakeVisible(b);b->getProperties().set("toolbarButton",true);
    }
    previous.setBounds(12,8,58,40); previous.setButtonText("<");previous.setTooltip("Previous preset");previous.setComponentID("previousPreset");
    next.setBounds(78,8,58,40); next.setButtonText(">");next.setTooltip("Next preset");next.setComponentID("nextPreset");
    presetMenuButton.setBounds(150,8,1230,40);presetMenuButton.setComponentID("presetMenu");
    presetMenuButton.setTooltip("Factory presets / categories");presetMenuButton.onClick=[this]{showPresetMenu();};
    bypass.setBounds(1394,8,198,40); bypass.setButtonText("BYPASS"); bypass.setClickingTogglesState(true);
    bypass.setComponentID("bypass");
    buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.state,"bypass",bypass));
    previous.onClick=[this]{processor.loadPreset((processor.getCurrentProgram()+processor.getNumPrograms()-1)%processor.getNumPrograms()); syncPreset();};
    next.onClick=[this]{processor.loadPreset((processor.getCurrentProgram()+1)%processor.getNumPrograms()); syncPreset();};
    setResizable(true,true); getConstrainer()->setFixedAspectRatio(static_cast<double>(artWidth)/(artHeight+toolbarHeight));
    setResizeLimits(802,518,1604,1036); setSize(1100,711);
    syncPreset(); timerCallback(); startTimerHz(30);
}
UnpluggedEditor::~UnpluggedEditor()
{
    stopTimer();if(presetMenuOpen)juce::PopupMenu::dismissAllActiveMenus();
    attachments.clear(); buttonAttachments.clear(); setLookAndFeel(nullptr);
}
juce::PopupMenu UnpluggedEditor::createPresetMenu() const
{
    juce::PopupMenu menu;menu.setLookAndFeel(&const_cast<Skin&>(skin));
    const auto& bank=unplugged::factoryPresets();const int current=processor.getCurrentProgram();
    const int neutral=151; // Stable legacy program index.
    menu.addItem(neutral+1,"Default / all modules bypassed",true,current==neutral);
    menu.addSeparator();
    const juce::StringArray groups {"Female Vocals","Male Vocals","Lead Vocals","Backing & Choir","Speech",
        "Drums","Percussion","Guitars & Bass","Strings","Keys & Synths","Brass & Winds","XML Reference"};
    for(const auto& group:groups) {
        juce::PopupMenu sub;
        for(std::size_t i=1;i<bank.size();++i) {
            const auto& p=bank[i];juce::String category=p.category;
            if(category=="Backing & Choirs")category="Backing & Choir";
            if(category=="Strings & Guitars") {
                const juce::String name=p.name;
                category=name.startsWith("Violin /")||name.startsWith("Cello /")||name.startsWith("String Section /")?"Strings":"Guitars & Bass";
            }
            if(category==group)sub.addItem(static_cast<int>(i)+1,p.name,true,current==static_cast<int>(i));
        }
        menu.addSubMenu(group,sub);
    }
    menu.addSeparator();menu.addItem(1,"Init / manual defaults",true,current==0);
    return menu;
}
void UnpluggedEditor::showPresetMenu()
{
    if(presetMenuOpen)return;
    presetMenuOpen=true;
    auto menu=createPresetMenu();
    auto options=juce::PopupMenu::Options().withTargetComponent(&presetMenuButton)
        .withMinimumWidth(430).withStandardItemHeight(35).withMaximumNumColumns(1);
    menu.showMenuAsync(options,[safe=juce::Component::SafePointer<UnpluggedEditor>(this)](int result) {
        if(safe==nullptr)return;
        safe->presetMenuOpen=false;
        if(result>0)safe->processor.loadPreset(result-1);
        safe->syncPreset();
    });
}
void UnpluggedEditor::syncPreset()
{
    const int index=processor.getCurrentProgram();
    if(index==shownPreset) return;
    shownPreset=index; const auto& entry=unplugged::factoryPresets()[static_cast<std::size_t>(index)];
    const juce::String name=entry.category=="Default"? (index==0?"Init":"Default"):juce::String(entry.name);
    presetMenuButton.setButtonText("A: "+name);
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
        colourButtons[i].getProperties().set("lit",mode!=0);
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
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff484a4b),0,0,juce::Colour(0xff151617),0,static_cast<float>(toolbarHeight),false));
    g.fillRect(0,0,artWidth,toolbarHeight);g.setColour(juce::Colour(0xffa0a09b));g.drawHorizontalLine(toolbarHeight-2,0,static_cast<float>(artWidth));
    g.setOrigin(0,toolbarHeight);
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
    controls.setBounds(0,toolbarHeight,artWidth,artHeight);
    controls.setTransform(juce::AffineTransform::scale(static_cast<float>(getWidth())/artWidth));
    toolbar.setBounds(0,0,artWidth,toolbarHeight);
    toolbar.setTransform(juce::AffineTransform::scale(static_cast<float>(getWidth())/artWidth));
}
void UnpluggedEditor::mouseDown(const juce::MouseEvent& event)
{
    const auto point=event.position*static_cast<float>(artWidth)/static_cast<float>(getWidth())-juce::Point<float>(0,static_cast<float>(toolbarHeight));
    if(juce::Rectangle<float>(1270,430,100,390).contains(point)) processor.clipped.store(false);
}
