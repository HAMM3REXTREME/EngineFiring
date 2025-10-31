#pragma once
class PostFilter {
  public:
    virtual ~PostFilter() = default;
    virtual float process(float in) = 0;
};