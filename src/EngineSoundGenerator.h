// EngineSoundGenerator.h

#pragma once

#include <vector>

#include "AudioVector.h"
#include "Engine.h"
#include "SoundGenerator.h"

class EngineSoundGenerator : public SoundGenerator {
  public:
    EngineSoundGenerator(const SoundBank &m_pistonClicks, const Engine &m_engine, float m_rpm, float m_max_amplitude = 0.5f, int m_sample_rate = 48000,
                         int m_channels = 1);

    // RPM controls
    void setRPM(float newRPM);
    float getRPM();

    // Audio sample generation
    void update();
    float getSample();

    void setAmplitude(float amp);
    float getAmplitude() const;

    void setNoteOffset(int offset); // VTEC kicked in, yo
    int getNoteOffset() const;

  private:
    const Engine &engine;
    const SoundBank &pistonClicks;
    float audioRpm; // Auditory rpm, refer to Engine.audioRpmFactor if it 'sounds' faster/slower than intended
    int noteOffset = 0;
    std::vector<std::pair<const std::vector<float> *, size_t>> active_firings;
    float interval;
    float interval_timer;
    int phase; // Decides which piston is next
    // Audio settings
    int sample_rate;
    int channels;
    float max_amplitude;
};