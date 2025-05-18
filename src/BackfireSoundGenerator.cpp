#include "Engine.h"
#include "SoundGenerator.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <vector>

#include "BackfireSoundGenerator.h"

float BackfireSoundGenerator::getAmplitude() const { return amplitude; }
void BackfireSoundGenerator::setAmplitude(float g) { amplitude = std::max(0.0f, g); }

BackfireSoundGenerator::BackfireSoundGenerator(float sampleRate) : sampleRate(sampleRate), amplitude(0.6f), intensity(0.0f) {
    srand(static_cast<unsigned>(time(nullptr)));
}

void BackfireSoundGenerator::setIntensity(float newIntensity) { intensity = std::clamp(newIntensity, 0.0f, 1.0f); }

float BackfireSoundGenerator::getIntensity() const { return intensity; }

void BackfireSoundGenerator::update() {
    // Random pop trigger based on intensity
    if (popQueue.empty() && ((float)rand() / (float)RAND_MAX) < intensity * 0.015f) {
        triggerPopBurst();
    }

    // Update active pops
    for (auto &pop : popQueue) {
        pop.envelope *= pop.decay;
    }

    // Remove finished pops
    popQueue.erase(std::remove_if(popQueue.begin(), popQueue.end(), [](const PopEvent &p) { return p.envelope < 0.001f; }), popQueue.end());
}

float BackfireSoundGenerator::getSample() {
    float sample = 0.0f;
    for (auto &pop : popQueue) {
        float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float freq = pop.freq;
        sample += pop.envelope * bandpass(noise, freq, 0.9f);
    }
    return std::clamp(sample * amplitude, -1.0f, 1.0f);
}

void BackfireSoundGenerator::triggerPopBurst() {
    int pops = 1 + rand() % 3; // 1–3 pops
    for (int i = 0; i < pops; ++i) {
        PopEvent pop;
        pop.envelope = 1.0f;
        pop.decay = std::exp(-5.0f / sampleRate) - 0.0001f * (rand() % 100) - 0.001f;
        pop.freq = 300.0f + rand() % 200; // between 300 and 500 Hz
        popQueue.push_back(pop);
    }
}

float BackfireSoundGenerator::bandpass(float input, float freq, float Q) {
    float omega = 2.0f * M_PI * freq / sampleRate;
    float alpha = sin(omega) / (2.0f * Q);

    float cos_omega = cos(omega);
    float norm = 1.0f / (1.0f + alpha);

    float a0 = alpha * norm;
    float a2 = -alpha * norm;
    float b1 = -2.0f * cos_omega * norm;
    float b2 = (1.0f - alpha) * norm;

    static float x1 = 0.0f, x2 = 0.0f, y1 = 0.0f, y2 = 0.0f;

    float y = a0 * input + a2 * x2 - b1 * y1 - b2 * y2;

    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = y;

    return y;
}