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
    AudioContext(const std::string &new_id, std::vector<SoundGenerator *> generators);
    AudioContext();
    std::string id;
    void update() override {}
    void addGenerator(SoundGenerator *generator);
    void addFilter(PostFilter* new_filter) { fx.addFilter(new_filter); }
    float getAllSamples();
    void getAllSamples(float *buffer, int numFrames, int numChannels);
    std::vector<SoundGenerator *> generators;
    EffectChain fx;
    float getSample() override;
    void setAmplitude(float amp) override;
    float getAmplitude() const override;
    std::string getInfo(int depth) const override;
    bool no_update = false; // Set to true to simply use as a effect wrapper/ adding a layer 

  private:
    float ctxAmplitude;
};