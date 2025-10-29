#pragma once
#include "PostFilter.h"
#include <algorithm>
#include <cmath>
class ForceSilenceFilter : public PostFilter {
    float process(float in) { return std::clamp(in, -1.0, 1.0); }
};