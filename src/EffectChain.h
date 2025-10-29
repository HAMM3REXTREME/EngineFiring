// EffectChain.h
#pragma once
#include "Biquad.h"
#include "PostFilter.h"
#include <memory>
#include <vector>

class EffectChain {
  public:
    void addFilter(std::unique_ptr<PostFilter> new_filter) { filters.push_back(std::move(new_filter)); }

    void removeFilter(size_t index) {
        if (index < filters.size())
            filters.erase(filters.begin() + index);
    }

    void clear() { filters.clear(); }

    // Process a single sample through all filters
    float process(float sample) {
        for (auto &filter : filters)
            sample = filter->process(sample);
        return sample;
    }

    // Accessors
    size_t size() const { return filters.size(); }
    PostFilter *operator[](size_t i) { return filters[i].get(); }
    const PostFilter *operator[](size_t i) const { return filters[i].get(); }

    // iterator support for range-for
    auto begin() { return filters.begin(); }
    auto end() { return filters.end(); }
    auto begin() const { return filters.begin(); }
    auto end() const { return filters.end(); }

  private:
    std::vector<std::unique_ptr<PostFilter>> filters;
};
