#pragma once
#include "PostFilter.h"
#include <cmath>
class SineClipper : public PostFilter {
    float process(float in) { return std::sin((M_PI / 2.0f) * in); }
};