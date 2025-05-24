#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Biquad.h"
#include "Car.h"
#include "Damper.h"
#include "EffectChain.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SimpleSoundGenerator.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include "TurboWhooshGenerator.h"

class AudioContext : public SoundGenerator {
  public:
    AudioContext(std::vector<SoundGenerator *> generators);
    AudioContext();
    void update() override {}
    void addGenerator(SoundGenerator *generator);
    float getAllSamples();
    void getAllSamples(float *buffer, int numFrames, int numChannels);
    std::vector<SoundGenerator *> generators;
    EffectChain fx;
    float getSample() override;
    void setAmplitude(float amp) override;
    float getAmplitude() const override;

  private:
    float ctxAmplitude;
};