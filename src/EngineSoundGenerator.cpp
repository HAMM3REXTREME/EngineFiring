#include "EngineSoundGenerator.h"
#include "Engine.h"

#include <algorithm>
#include <iostream>

EngineSoundGenerator::EngineSoundGenerator(const SoundBank &pistons_soundbank, const Engine &engine, float initial_rpm, float amplitude, int sample_rate)
    : m_pistons_soundbank(pistons_soundbank), m_engine(engine), m_interval_timer(0.0f), m_audio_bpm(engine.m_firing_per_rev * initial_rpm), m_fire_count(0),
      m_sample_rate(sample_rate), m_amplitude(amplitude) {
    m_interval = 60.0f / m_audio_bpm * sample_rate;
}

void EngineSoundGenerator::setRPM(float newRPM) {
    m_audio_bpm = m_engine.m_firing_per_rev * std::abs(newRPM) * m_engine.getCylinderCount();
    m_interval = 60.0f / m_audio_bpm * m_sample_rate; // Even fire interval (no offsets)
}

void EngineSoundGenerator::update() {
    m_interval_timer += 1.0f;
    if (m_interval_timer >= m_interval) {
        // Schedule new piston fire
        int piston_index = m_engine.m_firing_order[m_fire_count % m_engine.getCylinderCount()]; // Get the right piston
        active_firings.push_back({&m_pistons_soundbank.samples[piston_index + m_note_offset].samples,
                                  0}); // New firing event {pointer to sound samples for that piston, playback position of 0}
        m_fire_count++;                // We've fired a cylinder
        m_interval_timer -= m_engine.m_firing_interval_factors[piston_index] * m_interval; // Odd-firing factors: >1 = more time gap, <1 = less time gap
    }
}
void EngineSoundGenerator::setNoteOffset(int offset) { m_note_offset = offset; }
int EngineSoundGenerator::getNoteOffset() const { return m_note_offset; }
float EngineSoundGenerator::getRPM() { return m_audio_bpm / (m_engine.m_firing_per_rev * m_engine.getCylinderCount()); }
void EngineSoundGenerator::setAmplitude(float amp) { m_amplitude = amp; }
float EngineSoundGenerator::getAmplitude() const { return m_amplitude; }
float EngineSoundGenerator::getSample() {
    float sample = 0.0f;
    // Mix active firings
    for (auto it = active_firings.begin(); it != active_firings.end();) { // Go through the click samples, playback position of all active firings
        if (it->second < it->first->size()) { // If our playback position is less than the size of the click sample data, we're not done, add the click sample
                                              // value at the current playback position to OUR float sample then increment the playback position
            sample += (*(it->first))[it->second++] * m_amplitude;
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
    oss << "EngineSoundGenerator: using engine '" << m_engine.m_name << "' " << m_audio_bpm << " bpm,  note offset by " << m_note_offset << ", amplitude "
        << m_amplitude << "\n";
    return oss.str();
}
