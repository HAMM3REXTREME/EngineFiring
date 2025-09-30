// EffectChain.h
#pragma once
#include "Biquad.h"
#include "PostFilter.h"
#include <vector>

class EffectChain {
  public:
      ~EffectChain() {
        for (auto* f : filters) {
            delete f;
        }
        filters.clear();
    }
    size_t size() const { return filters.size(); }
    PostFilter* operator[](size_t i) { return filters[i]; }
    const PostFilter* operator[](size_t i) const { return filters[i]; }
    std::vector<PostFilter *> filters;
    void addFilter(PostFilter* new_filter) {
    filters.push_back(new_filter);
}

    void clear() { filters.clear(); }

    float process(float sample) {
        for (auto &filter : filters)
            sample = filter->process(sample);
        return sample;
    }
};
