#include "Engine.h"

#include <cmath>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "AudioVector.h"

void Engine::setIntervalsFromDegrees(const std::vector<float> &firing_intervals_degrees, float full_firing_degrees) {
    // Must add up to 720 degrees for a 4 stroke
    float sum_intervals = std::accumulate(firing_intervals_degrees.begin(), firing_intervals_degrees.end(), 0.0f);
    if (std::fabs(sum_intervals - full_firing_degrees) > 1e-4) {
        std::cerr << "Firing intervals dont add up to " << full_firing_degrees << " degrees\n";
    }
    if (firing_intervals_degrees.size() != getCylinderCount()) {
        std::cerr << "Count of firing intervals doesn't match cylinder count\n";
    }

    // Calculate relative interval for each cylinder
    float even_fire_interval = full_firing_degrees / getCylinderCount();
    m_firing_interval_factors.clear();
    std::cout << "Firing factors: ";
    for (float fire_interval : firing_intervals_degrees) {
        m_firing_interval_factors.push_back(fire_interval / even_fire_interval);
        std::cout << fire_interval / even_fire_interval << "  ";
    }
    std::cout << "\n";
}

Engine::Engine(std::string name, const std::vector<int> &firing_order, const std::vector<float> &firing_intervals_degrees, float firing_per_rev)
    : m_name(name), m_firing_order(firing_order), m_firing_per_rev(firing_per_rev) {
    std::cout << "New engine '" << m_name << "' with " << getCylinderCount() << " cylinders. ";
    setIntervalsFromDegrees(firing_intervals_degrees);
}

Engine::Engine(std::string name, const std::vector<int> &firing_order, float firing_per_rev)
    : m_name(name), m_firing_order(firing_order), m_firing_per_rev(firing_per_rev) {
    std::cout << "New engine '" << m_name << "' with " << getCylinderCount() << " cylinders (even firing).\n";
    m_firing_interval_factors.assign(getCylinderCount(), 1); // Even-firing
}

std::vector<int> Engine::getFiringOrderFromString(const std::string &firing_order_str) {
    std::vector<int> result;
    std::string cleaned_input = firing_order_str;

    // Replace any non-digit characters with spaces
    for (char &c : cleaned_input) {
        if (!std::isdigit(c)) {
            c = ' ';
        }
    }

    std::istringstream stream(cleaned_input);
    int number;
    while (stream >> number) {
        result.push_back(number - 1); // Convert from 1-based to 0-based
    }

    return result;
}
