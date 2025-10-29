#include "EngineSoundGenerator.h"
#include "Engine.h"

#include <algorithm>
#include <iostream>

EngineSoundGenerator::EngineSoundGenerator(const SoundBank &m_pistonClicks, const Engine &m_engine, float m_rpm, float m_max_amplitude, int m_sample_rate,
                                           int m_channels)
    : pistonClicks(m_pistonClicks), engine(m_engine), interval_timer(0.0f), audioBpm(engine.audioRpmFactor * m_rpm), fire_count(0), sample_rate(m_sample_rate),
      channels(m_channels), max_amplitude(m_max_amplitude) {
    interval = 60.0f / audioBpm * sample_rate;
}

void EngineSoundGenerator::setRPM(float newRPM) {
    audioBpm = engine.audioRpmFactor * std::abs(newRPM) * engine.getCylinderCount();
    interval = 60.0f / audioBpm * sample_rate; // Even fire interval (no offsets yet)
}

void EngineSoundGenerator::update() {
    interval_timer += 1.0f;
    if (interval_timer >= interval) {
        // Schedule new piston fire
        int piston_index = engine.firingOrder[fire_count % engine.getCylinderCount()]; // Get the right piston 
        active_firings.push_back({&pistonClicks.samples[piston_index + noteOffset].samples, 0}); // New firing event {reference to sound samples for that piston, playback position of 0}
        fire_count++; // We've fired a cylinder
        interval_timer -= engine.firingIntervalFactors[piston_index] * interval; // Odd-firing factors: >1 more time gap, <1 less time gap
    }
}
void EngineSoundGenerator::setNoteOffset(int offset) { noteOffset = offset; }
int EngineSoundGenerator::getNoteOffset() const { return noteOffset; }
float EngineSoundGenerator::getRPM() { return audioBpm / engine.audioRpmFactor; }
void EngineSoundGenerator::setAmplitude(float amp) { max_amplitude = amp; }
float EngineSoundGenerator::getAmplitude() const { return max_amplitude; }
float EngineSoundGenerator::getSample() {
    float sample = 0.0f;
    // Mix active firings
    for (auto it = active_firings.begin(); it != active_firings.end();) { // Go through the click samples, playback position of all active firings
        if (it->second < it->first->size()) { // If our playback position is less than the size of the click sample data, we're not done, add the click sample
                                              // value at the current playback position to OUR float sample then increment the playback position
            sample += (*(it->first))[it->second++] * max_amplitude;
            ++it;
        } else {
            it = active_firings.erase(it);
        }
    }
    return sample;
}

std::string EngineSoundGenerator::getInfo(int depth) const {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i) {
        oss << "    ";
    }
    oss << "EngineSoundGenerator: using engine '" << engine.name << "' " << audioBpm << " aRPM,  note offset by " << noteOffset << ", amplitude "
        << max_amplitude << "\n";
    return oss.str();
}
