#pragma once

#include "AudioVector.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include <cmath>
#include <cstddef>
#include <vector>

class SimpleSoundGenerator : public SoundGenerator {
  public:
    SimpleSoundGenerator(SoundBank &bank);

    // Call this to start playing a sample by its index in the SoundBank
    void startPlayback(size_t sampleIndex, bool loop = false);
    void stopPlayback();

    void update() override;
    float getSample() override;

    void setAmplitude(float newGain) override;
    float getAmplitude() const override;

    void setLooping(bool loop) { looping = loop; };
    bool isLooping() const { return looping; };

    std::string getInfo(int depth) const override;
    bool isPlaying() const;

  private:
    SoundBank &soundBank;
    int currentSampleIndex;  // -1 means no sample playing
    size_t playbackPosition; // Position within the current sample
    float gain;
    bool looping;
    bool paused;
  
};
