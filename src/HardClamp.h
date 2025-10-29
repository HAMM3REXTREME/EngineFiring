#pragma once
#include <algorithm>
#include <cmath>
#include "PostFilter.h"
class ForceSilenceFilter: public PostFilter {
    float process(float in){
        return std::clamp(in,-1.0,1.0);
    }
};