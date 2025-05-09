#pragma once

#include "SoundGenerator.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

class BackfireSoundGenerator : public SoundGenerator {
  public:
    BackfireSoundGenerator(float sampleRate);

    void update() override;

    float getSample() override;

    void setIntensity(float newIntensity);

    float getIntensity() const;

    void setAmplitude(float g) override;

    float getAmplitude() const override;

    void triggerPop() { triggerPopBurst(); }

  private:
    float sampleRate;
    float amplitude;
    float intensity;

    struct PopEvent {
        float envelope;
        float decay;
        float freq;
    };

    std::vector<PopEvent> popQueue;

    void triggerPopBurst();

    float bandpass(float input, float freq, float Q);
};
