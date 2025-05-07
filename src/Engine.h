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
    Engine(std::string m_name, const std::vector<int> &m_firingOrder, const std::vector<float> &m_degreesIntervals, float m_rpmFactor);
    Engine(std::string m_name, const std::vector<int> &m_firingOrder, float m_rpmFactor);

    void setIntervalsFromDegrees(const std::vector<float> &degreesInterval);
    static std::vector<int> getFiringOrderFromString(const std::string &firingString);

    std::string name;
    std::vector<int> firingOrder;
    std::vector<float> firingIntervalFactors;
    float audioRpmFactor;

    int getCylinderCount() const { return firingOrder.size(); };
};

#endif // ENGINE_H
