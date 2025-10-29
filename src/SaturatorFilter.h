#pragma once
#include <cmath>
#include "PostFilter.h"

// Utility clamp
inline float clampf(float x, float lo, float hi) {
    return std::fmax(lo, std::fmin(hi, x));
}

//
// 1️⃣ Warmth / Saturation
//    Smooth soft clipping — adds even harmonics, “tube” warmth.
//
class SaturatorFilter : public PostFilter {
public:
    explicit SaturatorFilter(float drive = 1.0f) : drive(drive) {}
    float process(float in) override {
        float x = in * drive;
        return std::tanh(x) / std::tanh(drive);
    }
private:
    float drive;
};

//
// 2️⃣ Punch Filter
//    Emphasizes transients: simple high-pass + limiter.
//    Adds "attack" to percussion or engine rev spikes.
//
class PunchFilter : public PostFilter {
public:
    PunchFilter(float cutoff = 80.0f, float sampleRate = 48000.0f)
        : cutoff(cutoff), sampleRate(sampleRate), prev(0.0f), hp(0.0f) {
        setCutoff(cutoff);
    }

    void setCutoff(float f) {
        alpha = std::exp(-2.0f * M_PI * f / sampleRate);
    }

    float process(float in) override {
        // 1-pole high-pass
        hp = alpha * (hp + in - prev);
        prev = in;
        // Slight saturation on peaks
        return std::tanh(hp * 1.5f);
    }
private:
    float cutoff, sampleRate, prev, hp, alpha;
};

//
// 3️⃣ Resonant Boost Filter
//    A tuned, damped resonator (like your SecondOrderFilter but as EQ).
//    Adds harmonic "body" or resonance around a target frequency.
//
class ResonantBoostFilter : public PostFilter {
public:
    ResonantBoostFilter(float frequency, float q, float sampleRate)
        : freq(frequency), q(q), sampleRate(sampleRate)
    {
        updateCoeffs();
    }

    void setParams(float frequency, float q) {
        freq = frequency; this->q = q; updateCoeffs();
    }

    float process(float in) override {
        float y = b0*in + b1*x1 + b2*x2 - a1*y1 - a2*y2;
        x2 = x1; x1 = in;
        y2 = y1; y1 = y;
        return y;
    }

private:
    float freq, q, sampleRate;
    float a1{}, a2{}, b0{}, b1{}, b2{};
    float x1{}, x2{}, y1{}, y2{};

    void updateCoeffs() {
        float w0 = 2.0f * M_PI * freq / sampleRate;
        float alpha = std::sin(w0) / (2.0f * q);
        float cosw0 = std::cos(w0);
        float boost = 2.0f; // how much to emphasize

        b0 = 1.0f + alpha * boost;
        b1 = -2.0f * cosw0;
        b2 = 1.0f - alpha * boost;
        float a0 = 1.0f + alpha / boost;
        a1 = -2.0f * cosw0;
        a2 = 1.0f - alpha / boost;

        b0/=a0; b1/=a0; b2/=a0; a1/=a0; a2/=a0;
    }
};

//
// 4️⃣ Harmonic Enricher
//    Subtle odd/even harmonic generator.
//    Great for complex tones — creates dynamic, natural richness.
//
class HarmonicEnricher : public PostFilter {
public:
    HarmonicEnricher(float mix = 0.5f, float drive = 1.2f)
        : mix(mix), drive(drive) {}

    float process(float in) override {
        float even = std::tanh(in * drive);
        float odd  = std::tanh(in * drive * 0.5f) * in; // slightly asymmetric
        float out = (even * 0.7f + odd * 0.3f);
        return in * (1.0f - mix) + out * mix;
    }

private:
    float mix, drive;
};
