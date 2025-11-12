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
    AudioContext(std::vector<SoundGenerator *> generators, float ctx_amp = 1.0f);
    AudioContext(float ctx_amp = 1.0f);
    ~AudioContext() {
        fx_chain.clear();
        sound_generators.clear();
    }
    void update() override {}
    void addGenerator(SoundGenerator *generator);
    void addFilter(std::unique_ptr<PostFilter> new_filter) { fx_chain.addFilter(std::move(new_filter)); }
    float getAllSamples();
    void getAllSamples(float *buffer, int numFrames, int numChannels);
    std::vector<SoundGenerator *> sound_generators;
    EffectChain fx_chain;
    float getSample() override;
    void setAmplitude(float amp) override;
    float getAmplitude() const override;
    std::string getInfo(int depth) const override;
    template <typename T> T *getFilterAs(size_t index) {
        if (index >= fx_chain.size())
            return nullptr;
        return dynamic_cast<T *>(fx_chain[index]);
    }

  private:
    float m_ctx_amplitude;
};