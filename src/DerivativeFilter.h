#pragma once
#include "PostFilter.h"

class DerivativeFilter : public PostFilter {
  public:
    DerivativeFilter(float sampleRate) : m_sampleRate(sampleRate), m_prevInput(0.0f), m_hasPrev(false) {}

    float process(float in) override {
        float out = 0.0f;

        if (m_hasPrev) {
            out = (in - m_prevInput) * m_sampleRate; // approximate derivative
        } else {
            m_hasPrev = true; // first sample, no derivative yet
        }

        m_prevInput = in;
        return out;
    }

    void reset() {
        m_hasPrev = false;
        m_prevInput = 0.0f;
    }

  private:
    float m_sampleRate; // samples per second
    float m_prevInput;
    bool m_hasPrev;
};
