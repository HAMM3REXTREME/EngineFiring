#pragma once
#include "PostFilter.h"
#include <cmath>
class TanhClipper : public PostFilter {
    float process(float in) { return std::tanh(in); }
};
