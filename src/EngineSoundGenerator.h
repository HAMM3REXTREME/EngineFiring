// EngineSoundGenerator.h

#pragma once

#include <vector>

#include "AudioVector.h"
#include "Engine.h"
#include "SoundGenerator.h"

class EngineSoundGenerator : public SoundGenerator {
  public:
    EngineSoundGenerator(const SoundBank &pistons_soundbank, const Engine &engine, float initial_rpm, float amplitude = 1.0f, int sample_rate = 48000);

    // RPM controls
    void setRPM(float new_rpm);
    float getRPM();

    // Audio sample generation
    void update() override;
    float getSample() override;

    // Linear Amplitude scaling
    void setAmplitude(float amp) override;
    float getAmplitude() const override;

    void setNoteOffset(int note_offset); // VTEC kicked in, yo
    int getNoteOffset() const;
    std::string getInfo(int depth) const override;

  private:
    // Important references
    const Engine &m_engine;               // Engine definition to use
    const SoundBank &m_pistons_soundbank; // SoundBank reference with each sound repping a piston

    // Controlled/modulated properties
    int m_note_offset = 0; // Index offset on SoundBank sample access
    float m_audio_bpm;     // Beats per minute

    // Internal timing
    float m_interval;       // Interval gap for a given rpm, sample rate condition
    int m_fire_count;       // Number of beats that have been fired
    float m_interval_timer; // Warning: Don't touch while the engine is running.
    // TODO: Replace this with a sensible option like a custom ring buffer
    std::vector<std::pair<const std::vector<float> *, size_t>> active_firings; // {pointer to sound samples for a piston, playback position}

    // Audio settings
    int m_sample_rate;
    float m_amplitude;
};