#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "AudioVector.h"  // Assuming AudioVector is defined elsewhere

class Engine {
   public:
    // Constructor
    Engine(std::string m_name, std::vector<AudioVector>& m_pistonClicks, const std::vector<int>& m_firingOrder, const std::vector<float>& m_degreesIntervals, float m_rpmFactor);
    Engine(std::string m_name, std::vector<AudioVector>& m_pistonClicks, const std::vector<int>& m_firingOrder, float m_rpmFactor);

    // Methods
    void setIntervalsFromDegrees(const std::vector<float>& degreesInterval);
    static std::vector<int> getFiringOrderFromString(const std::string& firingString);

    // Member variables
    std::string name;
    std::vector<AudioVector> pistonClicks;
    std::vector<int> firingOrder;
    std::vector<float> firingIntervalFactors;
    float audioRpmFactor;

    // Getter method for cylinder count
    int getCylinderCount() const { return firingOrder.size(); };
};

#endif  // ENGINE_H
