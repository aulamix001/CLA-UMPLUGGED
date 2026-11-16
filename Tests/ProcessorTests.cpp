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
        check(p.state.getParameter("balance")->getNormalisableRange().start==-6
            && p.state.getParameter("balance")->getNormalisableRange().end==6,"balance range");
        p.loadPreset(0);
        check(p.state.getRawParameterValue("predelay1")->load()==7
            && p.state.getRawParameterValue("predelay1On")->load()==0
            && p.state.getRawParameterValue("predelay2On")->load()==0,"manual predelay defaults");
        for(int i=0;i<p.getNumPrograms();++i) {
            p.loadPreset(i); const auto& s=unplugged::factoryPresets()[static_cast<std::size_t>(i)].settings;
            for(std::size_t j=0;j<6;++j) {
                check(std::abs(p.state.getRawParameterValue(UnpluggedProcessor::stages[j])->load()-s.amount[j])<.001f,"preset amount mapping");
                check(static_cast<int>(p.state.getRawParameterValue(juce::String(UnpluggedProcessor::stages[j])+"Colour")->load())==s.colour[j],"preset colour mapping");
            }
            const auto matches=[&](const juce::String& id,float value) {
                return std::abs(p.state.getRawParameterValue(id)->load()-value)<.001f;
            };
            if(i<152) check(matches("input",s.inputDb) && matches("output",s.outputDb),"preset gain mapping");
            check(matches("balance",s.balance),"preset balance mapping");
            check(matches("direct",s.direct?1.0f:0.0f) && matches("bypass",s.bypass?1.0f:0.0f),"preset bypass mapping");
            for(int j=0;j<2;++j) {
                const auto id="predelay"+juce::String(j+1);
                check(matches(id,static_cast<float>(s.preDelayNote[j])) && matches(id+"On",s.preDelayOn[j]?1.0f:0.0f),"preset predelay mapping");
            }
        }
        p.state.getParameter("input")->setValueNotifyingHost(p.state.getParameter("input")->convertTo0to1(3.2f));
        p.state.getParameter("output")->setValueNotifyingHost(p.state.getParameter("output")->convertTo0to1(-2.4f));
        p.loadPreset(153);
        check(std::abs(p.state.getRawParameterValue("input")->load()-3.2f)<.001f
            && std::abs(p.state.getRawParameterValue("output")->load()+2.4f)<.001f,"reference preset reset protected gains");
        auto legacy=p.state.copyState(); legacy.setProperty("version",1,nullptr);
        for(auto child:legacy) if(child.getProperty("id").toString()=="predelay1") child.setProperty("value",125.0f,nullptr);
        juce::MemoryBlock legacyData;
        juce::AudioProcessor::copyXmlToBinary(*legacy.createXml(),legacyData);
        p.setStateInformation(legacyData.getData(),static_cast<int>(legacyData.getSize()));
        check(p.state.getRawParameterValue("predelay1")->load()==7,"legacy millisecond state migration");
        std::cout<<"PASS automatic mapping, protected reference gains and legacy state migration\n";
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
        auto* ui=dynamic_cast<UnpluggedEditor*>(editor.get());check(ui!=nullptr,"unexpected editor type");
        auto* toolbar=editor->findChildWithID("topToolbar");check(toolbar!=nullptr,"top toolbar missing");
        auto* menuButton=toolbar->findChildWithID("presetMenu");check(menuButton!=nullptr,"preset selector missing");
        auto* canvas=editor->getChildComponent(0);
        check(canvas!=nullptr,"editor control container missing");
        const auto menuRect=editor->getLocalArea(menuButton,menuButton->getLocalBounds());
        const auto panelRect=editor->getLocalArea(canvas,canvas->getLocalBounds());
        check(menuRect.getBottom()<=panelRect.getY()+1,"presets are not above the artwork");
        auto menu=ui->createPresetMenu();
        std::vector<bool> seen(static_cast<std::size_t>(p.getNumPrograms()),false);
        juce::PopupMenu::MenuItemIterator all(menu,true);
        while(all.next()) {
            const int id=all.getItem().itemID;
            if(id>0) {check(id<=p.getNumPrograms(),"invalid preset menu ID");check(!seen[static_cast<std::size_t>(id-1)],"duplicate menu preset");seen[static_cast<std::size_t>(id-1)]=true;}
        }
        for(bool found:seen)check(found,"preset missing from category menu");
        juce::StringArray categoryNames;
        juce::PopupMenu::MenuItemIterator roots(menu);
        while(roots.next())if(roots.getItem().subMenu)categoryNames.add(roots.getItem().text);
        check(categoryNames==juce::StringArray{"Female Vocals","Male Vocals","Lead Vocals","Backing & Choir","Speech","Drums","Percussion","Guitars & Bass","Strings","Keys & Synths","Brass & Winds","XML Reference"},"menu category order differs from reference");
        p.loadPreset(151);
        for(const auto* stage:UnpluggedProcessor::stages)check(p.state.getRawParameterValue(juce::String(stage)+"Colour")->load()==0,"default does not bypass every module");
        p.loadPreset(0);
        // Render the real menu item painter over the real editor for visual review.
        auto menuPreview=screenshot.createCopy();
        {
        juce::Graphics mg(menuPreview);
        mg.addTransform(juce::AffineTransform::scale(static_cast<float>(editor->getWidth())/1604.0f));mg.setOrigin(150,54);
        auto& look=editor->getLookAndFeel();int menuHeight=4;
        juce::PopupMenu::MenuItemIterator sizes(menu);while(sizes.next())menuHeight+=sizes.getItem().isSeparator?10:35;
        look.drawPopupMenuBackground(mg,430,menuHeight);int itemY=2;
        juce::PopupMenu::MenuItemIterator rows(menu);
        while(rows.next()) {
            const auto& item=rows.getItem();const int itemHeight=item.isSeparator?10:35;
            look.drawPopupMenuItem(mg,{1,itemY,428,itemHeight},item.isSeparator,item.isEnabled,false,item.isTicked,item.subMenu!=nullptr,item.text,{},nullptr,nullptr);
            itemY+=itemHeight;
        }
        }
        check(menuPreview.getPixelAt(menuPreview.getWidth()-20,menuPreview.getHeight()-20)==screenshot.getPixelAt(screenshot.getWidth()-20,screenshot.getHeight()-20),"preset menu painted outside its bounds");
        const auto menuFile=juce::File::getCurrentWorkingDirectory().getChildFile("editor-menu-preview.png");
        auto menuStream=menuFile.createOutputStream();check(menuStream!=nullptr,"open menu preview");menuStream->setPosition(0);check(menuStream->truncate().wasOk(),"truncate menu preview");
        check(png.writeImageToStream(menuPreview,*menuStream),"write menu preview");menuStream.reset();
        std::cout<<"PASS top preset bar, complete category tree and all-modules bypass preset\n";
        auto* bass=dynamic_cast<juce::Slider*>(canvas->findChildWithID("bass"));
        check(bass!=nullptr,"bass control missing");
        bass->setValue(4.2,juce::sendNotificationSync);
        check(std::abs(p.state.getRawParameterValue("bass")->load()-4.2f)<.001f,"GUI-to-parameter mapping failed");
        auto* bassParameter=p.state.getParameter("bass");
        bassParameter->setValueNotifyingHost(bassParameter->convertTo0to1(-3.0f));
        check(std::abs(bass->getValue()+3.0)<.001,"automation-to-GUI mapping failed");
        p.loadPreset(0);
        std::cout<<"PASS bidirectional GUI and host parameter mapping\n";
        auto* predelayKnob=dynamic_cast<juce::Slider*>(canvas->findChildWithID("predelay1"));
        auto* predelaySwitch=dynamic_cast<juce::TextButton*>(canvas->findChildWithID("predelay1On"));
        auto* colour=dynamic_cast<juce::TextButton*>(canvas->findChildWithID("bassColour"));
        check(predelayKnob&&predelaySwitch&&colour,"realistic controls missing");
        predelayKnob->setValue(10,juce::sendNotificationSync);check(p.state.getRawParameterValue("predelay1")->load()==10,"knob mapping failed");
        const auto lightPoint=editor->getLocalPoint(predelaySwitch,predelaySwitch->getLocalBounds().getCentre());
        predelaySwitch->setToggleState(false,juce::sendNotificationSync);check(p.state.getRawParameterValue("predelay1On")->load()==0,"off switch mapping failed");
        const auto offPixel=editor->createComponentSnapshot(editor->getLocalBounds()).getPixelAt(lightPoint.x,lightPoint.y);
        predelaySwitch->setToggleState(true,juce::sendNotificationSync);check(p.state.getRawParameterValue("predelay1On")->load()==1,"on switch mapping failed");
        const auto onPixel=editor->createComponentSnapshot(editor->getLocalBounds()).getPixelAt(lightPoint.x,lightPoint.y);
        check(onPixel.getBrightness()>offPixel.getBrightness() && static_cast<int>(onPixel.getGreen())-onPixel.getBlue()>25,"illuminated button does not reflect on/off state");
        auto* secondKnob=dynamic_cast<juce::Slider*>(canvas->findChildWithID("predelay2"));
        auto* secondSwitch=dynamic_cast<juce::TextButton*>(canvas->findChildWithID("predelay2On"));
        check(secondKnob&&secondSwitch,"second predelay control missing");
        predelayKnob->setValue(13,juce::sendNotificationSync);secondKnob->setValue(8,juce::sendNotificationSync);
        secondSwitch->setToggleState(false,juce::sendNotificationSync);
        auto controlsPreview=editor->createComponentSnapshot(editor->getLocalArea(canvas,juce::Rectangle<int>{72,190,295,150}),true,1.5f);
        const auto controlsFile=juce::File::getCurrentWorkingDirectory().getChildFile("editor-predelays-preview.png");
        auto controlsStream=controlsFile.createOutputStream();check(controlsStream!=nullptr,"open controls preview");controlsStream->setPosition(0);check(controlsStream->truncate().wasOk(),"truncate controls preview");
        check(png.writeImageToStream(controlsPreview,*controlsStream),"write controls preview");controlsStream.reset();
        for(int mode:{2,3,0,1}) {colour->onClick();check(static_cast<int>(p.state.getRawParameterValue("bassColour")->load())==mode,"colour button cycle failed");}
        const auto* forward=dynamic_cast<juce::TextButton*>(toolbar->findChildWithID("nextPreset"));
        check(forward!=nullptr,"next preset button missing");forward->onClick();check(p.getCurrentProgram()==1,"top bar navigation failed");
        for(int width:{802,1100,1604}) {editor->setSize(width,static_cast<int>(std::lround(width*1036.0/1604.0)));check(editor->createComponentSnapshot(editor->getLocalBounds()).isValid(),"resize rendering failed");}
        p.loadPreset(0);
        std::cout<<"PASS knob, illuminated switches, colour cycling, preset navigation and resized rendering\n";
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
