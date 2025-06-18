#include "EngineSoundGenerator.h"

#include <algorithm>

EngineSoundGenerator::EngineSoundGenerator(const SoundBank &m_pistonClicks, const Engine &m_engine, float m_rpm, float m_max_amplitude, int m_sample_rate,
                                           int m_channels)
    : pistonClicks(m_pistonClicks), engine(m_engine), interval_timer(0.0f), audioRpm(engine.audioRpmFactor * m_rpm), phase(0), sample_rate(m_sample_rate),
      channels(m_channels), max_amplitude(m_max_amplitude) {
    interval = 60.0f / audioRpm * sample_rate;
}

void EngineSoundGenerator::setRPM(float newRPM) {
    audioRpm = engine.audioRpmFactor * std::abs(newRPM);
    interval = 60.0f / audioRpm * sample_rate;
}

void EngineSoundGenerator::update() {
    interval_timer += 1.0f;
    if (interval_timer >= interval) {
        // Schedule new piston fire
        int piston_index = engine.firingOrder[phase % engine.getCylinderCount()];
        active_firings.push_back({&pistonClicks.samples[piston_index + noteOffset].samples, 0});
        phase++;
        // std::string firingVisual(engine.getCylinderCount(), '-');
        // firingVisual[piston_index] = 'O';
        // std::cout << firingVisual << " | ";
        // std::cout << "Interval timer -= " <<
        // engine.firingIntervalFactors[piston_index] * interval << "\n";
        interval_timer -= engine.firingIntervalFactors[piston_index] * interval; // Larger number = slower, 0.1 etc. = faster
    }
}
void EngineSoundGenerator::setNoteOffset(int offset) { noteOffset = offset; }
int EngineSoundGenerator::getNoteOffset() const { return noteOffset; }
float EngineSoundGenerator::getRPM() { return audioRpm / engine.audioRpmFactor; }
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
    return std::clamp(sample, -1.0f, 1.0f);
}
