#pragma once
#include "PostFilter.h"
#include <algorithm>
#include <cmath>
class CubicClipper : public PostFilter {
    float process(float in) { return in - (1.0f/3.0f) * in*in*in; }
};