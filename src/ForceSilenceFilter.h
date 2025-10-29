#pragma once
#include <cmath>
#include "PostFilter.h"
class ForceSilenceFilter: public PostFilter {
    float process(float in){
        return 0;
    }
};