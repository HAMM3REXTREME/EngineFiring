#pragma once

#include "SoundGenerator.h"
#include "AudioVector.h"
#include <cmath>
#include <vector>
#include <cstddef>
#include "SoundBank.h"

class SimpleSoundGenerator : public SoundGenerator {
public:
    SimpleSoundGenerator(SoundBank& bank)
        : soundBank(bank), currentSampleIndex(-1), playbackPosition(0), gain(1.0f)
    {}

    // Call this to start playing a sample by its index in the SoundBank
    void startPlayback(size_t sampleIndex) {
        if (sampleIndex < soundBank.samples.size()) {
            currentSampleIndex = static_cast<int>(sampleIndex);
            playbackPosition = 0;
        }
    }

    void update() override {
        // Just advances the playback position if a sample is active
        if (isPlaying()) {
            ++playbackPosition;
            if (playbackPosition >= soundBank.samples[currentSampleIndex].samples.size()) {
                currentSampleIndex = -1;
            }
        }
    }

    float getSample() override {
        if (isPlaying()) {
            return gain * soundBank.samples[currentSampleIndex].samples[playbackPosition];
        }
        return 0.0f; // Silence when not playing
    }

    void setAmplitude(float newGain) override {
        gain = std::fmax(0.0f, newGain);
    }

    float getAmplitude() const override {
        return gain;
    }

private:
    SoundBank& soundBank;
    int currentSampleIndex;      // -1 means no sample playing
    size_t playbackPosition;     // Position within the current sample
    float gain;

    bool isPlaying() const {
        return currentSampleIndex >= 0 &&
               currentSampleIndex < static_cast<int>(soundBank.samples.size()) &&
               playbackPosition < soundBank.samples[currentSampleIndex].samples.size();
    }
};
