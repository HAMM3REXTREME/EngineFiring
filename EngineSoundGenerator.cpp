#include "EngineSoundGenerator.h"

#include <algorithm>

EngineSoundGenerator::EngineSoundGenerator(const Engine& m_engine, float m_rpm, int m_sample_rate, int m_channels, float m_max_amplitude) : engine(m_engine), interval_timer(0.0f), audioRpm(engine.audioRpmFactor * m_rpm), phase(0) ,     sample_rate(m_sample_rate),   // Move to initializer list
channels(m_channels),  
max_amplitude(m_max_amplitude) {
    interval = 60.0f / audioRpm * sample_rate;
}

void EngineSoundGenerator::setRPM(float newRPM) {
    audioRpm = engine.audioRpmFactor * newRPM;
    interval = 60.0f / audioRpm * sample_rate;
}

void EngineSoundGenerator::update() {
    interval_timer += 1.0f;
    if (interval_timer >= interval) {
        // Schedule new piston fire
        int piston_index = engine.firingOrder[phase % engine.getCylinderCount()];
        active_firings.push_back({&engine.pistonClicks[piston_index].samples, 0});
        phase++;
        //std::string firingVisual(engine.getCylinderCount(), '-');
        //firingVisual[piston_index] = 'O';
        //std::cout << firingVisual << " | ";
        //std::cout << "Interval timer -= " << engine.firingIntervalFactors[piston_index] * interval << "\n";
        interval_timer -= engine.firingIntervalFactors[piston_index] * interval;  // Larger number = slower, 0.1 etc. = faster
    }
}
float EngineSoundGenerator::getRPM() { return audioRpm / engine.audioRpmFactor; }
float EngineSoundGenerator::getSample() {
    float sample = 0.0f;
    // Mix active firings
    for (auto it = active_firings.begin(); it != active_firings.end();) {
        if (it->second < it->first->size()) {
            sample += (*(it->first))[it->second++] * max_amplitude;
            ++it;
        } else {
            it = active_firings.erase(it);
        }
    }
    return std::clamp(sample, -1.0f, 1.0f);
}
