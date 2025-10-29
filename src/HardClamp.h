#pragma once
#include "PostFilter.h"
#include <algorithm>
#include <cmath>
class HardClamp : public PostFilter {
    float process(float in) { return std::clamp(in, -1.0f, 1.0f); }
};