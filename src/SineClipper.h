#pragma once
#include <cmath>
#include "PostFilter.h"
class SineClipper: public PostFilter {
    float process(float in){
        return std::sin((M_PI / 2.0f) * in);
    }
};