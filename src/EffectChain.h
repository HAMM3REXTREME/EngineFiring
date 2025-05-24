// EffectChain.h
#pragma once
#include "Biquad.h"
#include <vector>

class EffectChain {
  public:
    std::vector<Biquad> biquads;
    void addFilter(const Biquad &biquad) { biquads.push_back(biquad); }

    void clear() { biquads.clear(); }

    float process(float sample) {
        for (auto &biquad : biquads)
            sample = biquad.process(sample);
        return sample;
    }
};
