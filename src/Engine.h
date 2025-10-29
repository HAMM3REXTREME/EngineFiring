#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "AudioVector.h"
#include "SoundBank.h"

class Engine {
  public:
    Engine(std::string name, const std::vector<int> &firing_order, const std::vector<float> &firing_intervals_degrees, float firing_per_rev = 0.5);
    Engine(std::string name, const std::vector<int> &firing_order, float firing_per_rev = 0.5);

    void setIntervalsFromDegrees(const std::vector<float> &firing_intervals_degrees, float full_firing_degrees = 720.0f);
    static std::vector<int> getFiringOrderFromString(const std::string &firing_order_str);

    std::string m_name;
    std::vector<int> m_firing_order;
    std::vector<float> m_firing_interval_factors;
    float m_firing_per_rev; // 4-stroke: 360/720 = 0.5

    int getCylinderCount() const { return m_firing_order.size(); };
};

#endif // ENGINE_H
