#pragma once

#include "SoundGenerator.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

class TurboWhooshGenerator : public SoundGenerator {
  public:
    TurboWhooshGenerator(float sampleRate);

    void update() override;

    float getSample() override;

    void setIntensity(float newIntensity);
    float getIntensity() const;

    // Set the gain for overall volume control
    void setAmplitude(float newGain) override;

    // Get the current gain value
    float getAmplitude() const override;

  private:
    float sampleRate;
    float intensity;
    float phase;
    float amplitude; // Gain adjustment for overall volume control

    // Filter state
    float a0, a1, a2, b1, b2;
    float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    void resetFilter();

    float whiteNoise();

    void computeBandpassCoeffs(float freq, float Q);

    float processBandpass(float input);
};
