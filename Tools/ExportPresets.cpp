#include "Presets.h"
#include <iostream>
#include <iomanip>
int main()
{
    const char* stages[]{"bass","treble","compress","reverb1","reverb2","delay"};
    const auto& bank=unplugged::factoryPresets();
    std::cout<<"{\n  \"schemaVersion\": 1,\n  \"plugin\": \"Unplugged Study\",\n  \"presets\": [\n";
    for(std::size_t i=0;i<bank.size();++i) {
        const auto& p=bank[i]; const auto& s=p.settings;
        std::cout<<"    {\"index\": "<<i<<", \"category\": \""<<p.category<<"\", \"name\": \""<<p.name<<"\", \"parameters\": {";
        std::cout<<"\"input\": "<<s.inputDb<<", \"output\": "<<s.outputDb<<", \"balance\": "<<s.balance;
        for(int j=0;j<6;++j) std::cout<<", \""<<stages[j]<<"\": "<<s.amount[j]<<", \""<<stages[j]<<"Colour\": "<<s.colour[j];
        for(int j=0;j<2;++j) std::cout<<", \"predelay"<<j+1<<"\": "<<s.preDelayMs[j]<<", \"predelay"<<j+1<<"On\": "<<(s.preDelayOn[j]?"true":"false");
        std::cout<<", \"direct\": "<<(s.direct?"true":"false")<<", \"bypass\": "<<(s.bypass?"true":"false")<<"}}"<<(i+1<bank.size()?",":"")<<'\n';
    }
    std::cout<<"  ]\n}\n";
}
