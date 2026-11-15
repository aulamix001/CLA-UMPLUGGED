#pragma once
#include "DspEngine.h"
#include <string>
#include <vector>

namespace unplugged
{
struct Preset { std::string category, name; Settings settings; };
const std::vector<Preset>& factoryPresets();
}
