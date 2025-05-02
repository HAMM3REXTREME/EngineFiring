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
    Engine(std::string m_name, std::vector<AudioVector>& m_pistonClicks, 
           const std::vector<int>& m_firingOrder, const std::vector<float>& m_degreesIntervals);
           Engine(std::string m_name, std::vector<AudioVector>& m_pistonClicks, const std::vector<int>& m_firingOrder);

    // Methods
    void setIntervalsFromDegrees(const std::vector<float>& degreesInterval);
    static std::vector<int> getFiringOrderFromString(const std::string& firingString);

    // Member variables
    std::string name;
    std::vector<AudioVector> pistonClicks;
    std::vector<int> firingOrder;
    std::vector<float> firingIntervalFactors;

    // Getter method for cylinder count
    int getCylinderCount() {return firingOrder.size();};
    // TODO: Add sound RPM factor here...
};

#endif  // ENGINE_H
