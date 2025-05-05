// EngineSoundGenerator.h

#pragma once

#include <vector>

#include "AudioVector.h"
#include "Engine.h"

class EngineSoundGenerator {
  public:
    EngineSoundGenerator(const Engine &m_engine, float m_rpm, int m_sample_rate = 44100, int m_channels = 1, float m_max_amplitude = 0.5f);

    // RPM controls
    void setRPM(float newRPM);
    float getRPM();

    // Audio sample generation
    void update();
    float getSample();

  private:
    const Engine &engine;
    float audioRpm;
    std::vector<std::pair<const std::vector<float> *, size_t>> active_firings;
    float interval;
    float interval_timer;
    int phase;
    // Audio settings
    int sample_rate;
    int channels;
    float max_amplitude;
};