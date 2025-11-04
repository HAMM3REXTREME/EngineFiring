#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "PostFilter.h"

// A smoother, musical chorus filter to add warmth and width
class ChorusFilter : public PostFilter {
public:
    ChorusFilter(float rateHz = 0.3f, float depthMs = 12.0f, float mix = 0.35f, float sampleRate = 48000.0f)
        : rate(rateHz), depthMs(depthMs), mix(mix), sampleRate(sampleRate),
          delayIndex(0), phase(0.0f)
    {
        const float maxDelayMs = depthMs * 2.0f + 10.0f; // safety headroom
        int maxSamples = static_cast<int>(maxDelayMs * 0.001f * sampleRate);
        delayBuffer.assign(maxSamples, 0.0f);
    }

    float process(float in) override {
        // --- LFO for smooth modulation ---
        float lfo = std::sin(phase * 2.0f * M_PI) * 0.5f + 0.5f;  // 0–1
        float delayMs = 5.0f + lfo * depthMs;                     // base + mod
        float delaySamples = delayMs * 0.001f * sampleRate;

        // --- Write input into circular buffer ---
        delayBuffer[delayIndex] = in;

        // --- Read delayed sample (with safe interpolation) ---
        float readIndex = delayIndex - delaySamples;
        while (readIndex < 0) readIndex += delayBuffer.size();

        int i1 = static_cast<int>(readIndex);
        int i2 = (i1 + 1) % delayBuffer.size();
        float frac = readIndex - i1;

        float delayed = delayBuffer[i1] * (1.0f - frac) + delayBuffer[i2] * frac;

        // --- Update pointers ---
        delayIndex = (delayIndex + 1) % delayBuffer.size();
        phase += rate / sampleRate;
        if (phase >= 1.0f) phase -= 1.0f;

        // --- Mix wet & dry ---
        return (1.0f - mix) * in + mix * delayed;
    }

private:
    std::vector<float> delayBuffer;
    int delayIndex;
    float rate;       // modulation speed
    float depthMs;    // modulation depth (ms)
    float mix;        // wet/dry
    float sampleRate;
    float phase;
};
