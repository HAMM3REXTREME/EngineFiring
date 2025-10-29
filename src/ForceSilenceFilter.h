#pragma once
#include "PostFilter.h"
#include <cmath>
class ForceSilenceFilter : public PostFilter {
    float process(float in) { return 0; }
};