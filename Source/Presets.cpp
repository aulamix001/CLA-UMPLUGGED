#include "Presets.h"
#include <algorithm>

namespace unplugged
{
const std::vector<Preset>& factoryPresets()
{
    // Starting points for normalised recordings, not automatic source detection.
    static const std::vector<Preset> bank=[] {
        struct Recipe {
            const char* category; const char* name;
            std::array<float,6> amount;
            std::array<int,6> colour;
            float pre1,pre2,output;
            bool direct=true;
        };
        const Recipe recipes[] {
            {"Female Vocals","Female / Clean Air",{-2,3,1,-5,-9,-9},{2,3,1,2,1,0},28,10,-2},
            {"Female Vocals","Female / Pop Presence",{-3,4,4,-5,-8,-6},{2,2,2,2,2,2},35,18,-3},
            {"Female Vocals","Female / Intimate",{1,1,-2,-8,-10,-10},{2,2,1,1,0,0},16,16,-1},
            {"Female Vocals","Female / Dream Chamber",{-3,2,0,2,-3,-2},{2,3,1,3,3,3},60,32,-3},
            {"Male Vocals","Male / Warm Lead",{2,2,2,-6,-8,-8},{2,2,1,2,1,1},30,12,-3},
            {"Male Vocals","Male / Rock Edge",{-1,4,5,-6,-8,-5},{3,1,2,2,1,1},22,10,-4},
            {"Male Vocals","Male / Low Baritone",{2,3,1,-8,-10,-9},{1,2,1,1,0,1},20,10,-3},
            {"Male Vocals","Male / Ballad",{1,2,2,-1,-4,-4},{2,3,1,3,2,3},48,24,-3},
            {"Backing & Choirs","Backing / Female Stack",{-4,-1,2,-1,-4,-6},{2,2,1,3,2,2},15,10,-4},
            {"Backing & Choirs","Backing / Male Stack",{-2,-2,3,-2,-4,-6},{2,2,1,2,2,2},12,10,-4},
            {"Backing & Choirs","Choir / Concert Hall",{-2,1,-2,3,-2,-8},{2,3,1,3,3,0},35,20,-3},
            {"Lead Vocals","Lead / Modern Pop",{-2,4,4,-4,-7,-4},{2,2,2,2,2,2},40,20,-4},
            {"Lead Vocals","Lead / Vintage Slap",{2,-1,2,-6,-9,0},{2,1,1,1,0,1},10,10,-3},
            {"Lead Vocals","Lead / Long Echo",{-2,2,3,-3,-6,2},{2,3,2,3,2,3},55,25,-4},
            {"Speech","Podcast / Close & Warm",{2,2,1,-10,-10,-10},{2,2,1,0,0,0},16,16,-3},
            {"Speech","Narration / Natural",{1,1,-2,-10,-10,-10},{2,2,1,0,0,0},16,16,-2},
            {"Speech","Broadcast / Radio",{4,3,4,-10,-10,-10},{1,1,2,0,0,0},16,16,-5},
            {"Drums","Snare / Rock Punch",{-1,5,5,-6,-2,-10},{3,1,2,1,2,0},12,4,-4},
            {"Drums","Snare / Pop Snap",{-2,3,3,-7,-5,-10},{3,2,2,1,1,0},8,0,-3},
            {"Drums","Kick / Deep Weight",{5,2,2,-10,-10,-10},{1,1,2,0,0,0},0,0,-5},
            {"Drums","Kick / Beater Click",{2,5,4,-10,-10,-10},{2,1,3,0,0,0},0,0,-5},
            {"Drums","Toms / Tight Room",{3,3,3,-8,-5,-10},{2,1,2,1,1,0},5,0,-4},
            {"Drums","Overheads / Controlled Air",{-4,3,1,-9,-7,-10},{2,3,1,1,1,0},0,0,-3},
            {"Drums","Room / Wall Crush",{1,2,9,-4,-3,-10},{2,1,3,2,2,0},0,0,-7},
            {"Drums","Parallel Bus / Crush",{3,4,10,-8,-8,-10},{1,1,3,1,1,0},0,0,-8},
            {"Percussion","Pandeiro / Spark",{-3,4,1,-7,-8,-10},{2,2,2,1,1,0},6,2,-3},
            {"Percussion","Shaker / Soft Pocket",{-5,1,-1,-8,-7,-9},{2,3,1,1,1,2},0,0,-2},
            {"Percussion","Congas / Live Room",{2,2,2,-5,-6,-8},{2,1,1,1,2,1},10,4,-3},
            {"Percussion","Cajon / Wood & Snap",{3,3,3,-7,-8,-10},{1,1,2,1,1,0},5,0,-4},
            {"Strings & Guitars","Acoustic Steel / Pick Air",{-1,4,1,-5,-7,-9},{2,2,1,2,1,0},18,8,-3},
            {"Strings & Guitars","Acoustic Nylon / Warm",{2,1,-1,-4,-7,-10},{2,2,1,2,1,0},22,12,-2},
            {"Strings & Guitars","Rhythm Guitar / Bite",{2,4,2,-8,-9,-10},{3,1,2,1,0,0},8,0,-4},
            {"Strings & Guitars","Lead Guitar / Sustain Echo",{1,3,6,-4,-6,1},{3,1,2,3,2,2},32,12,-5},
            {"Strings & Guitars","Bass / Finger Definition",{4,3,5,-10,-10,-10},{2,1,3,0,0,0},0,0,-6},
            {"Strings & Guitars","Bass / Pick Weight",{5,4,4,-10,-10,-10},{1,1,3,0,0,0},0,0,-6},
            {"Strings & Guitars","Violin / Silky Hall",{-2,2,-2,-1,-6,-10},{2,2,1,3,2,0},30,12,-2},
            {"Strings & Guitars","Cello / Warm Chamber",{3,1,-1,0,-6,-10},{2,2,1,3,2,0},35,15,-3},
            {"Strings & Guitars","String Section / Stage",{1,2,-3,2,-3,-9},{2,2,1,3,3,0},40,18,-3},
            {"Keys & Synths","Piano / Mix Glue",{1,3,1,-5,-7,-9},{2,2,1,2,1,0},25,10,-3},
            {"Keys & Synths","Rhodes / Velvet",{3,0,1,-3,-5,-5},{2,2,1,2,2,2},20,10,-3},
            {"Keys & Synths","Wurlitzer / Bark",{1,4,4,-6,-8,-5},{3,1,2,1,1,1},12,5,-4},
            {"Keys & Synths","Synth Lead / Edge Echo",{-1,4,3,-6,-7,2},{2,1,2,2,1,2},20,8,-4},
            {"Keys & Synths","Arpeggio / Tempo Eighth",{-2,3,1,-4,-6,3},{2,2,1,2,2,2},18,8,-4},
            {"Keys & Synths","Pad / Deep Atmosphere",{1,-1,-3,4,1,0},{2,3,1,3,3,3},60,40,-4},
            {"Keys & Synths","Hammond / Grit & Body",{3,4,4,-6,-8,-8},{2,1,2,1,1,1},10,5,-5},
            {"Brass & Winds","Sax / Smoky Slap",{2,2,3,-5,-7,-2},{2,2,1,2,1,1},20,8,-4},
            {"Brass & Winds","Trumpet / Controlled Shine",{-2,2,4,-4,-6,-6},{2,2,2,2,2,1},25,10,-4},
            {"Brass & Winds","Trombone / Warm Space",{3,1,3,-4,-6,-7},{2,2,1,2,2,1},28,12,-4},
            {"Brass & Winds","Harmonica / Front Row",{1,4,6,-7,-9,-4},{3,1,2,1,1,1},8,0,-5},
            {"Brass & Winds","Accordion / Folk Room",{1,2,2,-5,-7,-8},{2,2,1,1,2,1},15,6,-3}
        };
        std::vector<Preset> result;
        result.push_back({"Default","Init",Settings{}});
        for(const auto& r:recipes) {
            Settings s; s.amount=r.amount; s.colour=r.colour;
            s.preDelayMs={r.pre1,r.pre2}; s.outputDb=r.output; s.direct=r.direct;
            result.push_back({r.category,r.name,s});
            // Two explicit alternative spaces per source; no invented pitch/stereo stage.
            Settings dry=s;
            dry.colour[3]=dry.colour[4]=dry.colour[5]=0;
            dry.amount[3]=dry.amount[4]=dry.amount[5]=-10;
            result.push_back({r.category,std::string(r.name)+" / Dry",dry});
            Settings ambience=s;
            ambience.colour[3]=2; ambience.colour[4]=2;
            ambience.amount[3]=std::max(-2.0f,s.amount[3]+3);
            ambience.amount[4]=std::max(-5.0f,s.amount[4]+2);
            ambience.colour[5]=2; ambience.amount[5]=std::max(-5.0f,s.amount[5]+3);
            ambience.preDelayMs[0]=std::min(160.0f,s.preDelayMs[0]+20);
            ambience.outputDb=std::max(-10.0f,s.outputDb-1);
            result.push_back({r.category,std::string(r.name)+" / Ambient",ambience});
        }
        // Append only: existing DAW program indices remain compatible with v0.1.0.
        Settings neutral;neutral.colour.fill(0);neutral.preDelayOn.fill(false);
        result.push_back({"Default","Default / all modules bypassed",neutral});
        return result;
    }();
    return bank;
}
}
