#include "Engine.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "AudioVector.h"

void Engine::setIntervalsFromDegrees(const std::vector<float> &degreesInterval) {
    // Must add up to 720 degrees or it won't sound right
    float sumIntervals = std::accumulate(degreesInterval.begin(), degreesInterval.end(), 0.0f);
    if (std::fabs(sumIntervals - 720.0f) > 1e-4) {
        std::cerr << "Firing order doesn't make any sense\n";
    }
    if (degreesInterval.size() != getCylinderCount()) {
        std::cerr << "Firing order list doesn't match cylinder count\n";
    }
    // Calculate relative interval for each cylinder
    float evenFireInterval = 720.0f / getCylinderCount();
    firingIntervalFactors.clear();
    std::cout << name << " -> Firing factors (" << getCylinderCount() << " cylinders): ";
    for (float fireInterval : degreesInterval) {
        firingIntervalFactors.push_back(fireInterval / evenFireInterval);
        std::cout << "  " << fireInterval / evenFireInterval << "  ";
    }
    std::cout << "\n";
}
Engine::Engine(std::string m_name, std::vector<AudioVector> &m_pistonClicks, const std::vector<int> &m_firingOrder,
               const std::vector<float> &m_degreesIntervals, float m_rpmFactor)
    : name(m_name), pistonClicks(m_pistonClicks), firingOrder(m_firingOrder), audioRpmFactor(m_rpmFactor) {
    std::cout << "Generating new engine with" << getCylinderCount() << " cylinders...\n";
    setIntervalsFromDegrees(m_degreesIntervals);
}
Engine::Engine(std::string m_name, std::vector<AudioVector> &m_pistonClicks, const std::vector<int> &m_firingOrder, float m_rpmFactor)
    : name(m_name), pistonClicks(m_pistonClicks), firingOrder(m_firingOrder), audioRpmFactor(m_rpmFactor) {
    std::cout << "Generating new engine with" << getCylinderCount() << " cylinders (even firing)...\n";
    firingIntervalFactors.assign(getCylinderCount(), 1); // Even-firing
}
// Assuming we numbered the cylinders in each bank sequentially (Audi, Ford,
// Porsche) - might sound sonically different for GM numbering
std::vector<int> Engine::getFiringOrderFromString(const std::string &firingString) {
    std::vector<int> result;
    std::string cleanedInput = firingString;

    // Replace any non-digit characters with spaces
    for (char &c : cleanedInput) {
        if (!std::isdigit(c)) {
            c = ' ';
        }
    }

    std::istringstream stream(cleanedInput);
    int number;
    while (stream >> number) {
        result.push_back(number - 1); // Convert from 1-based to 0-based
    }

    return result;
}
