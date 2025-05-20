#include "AudioContext.h"

AudioContext::AudioContext(std::vector<SoundGenerator *> generators) : generators(generators) {}
AudioContext::AudioContext() {}

void AudioContext::addGenerator(SoundGenerator *generator) {
    generators.push_back(generator);
}

float AudioContext::getAllSamples() {
    float sample = 0.0f;
    for (auto *gen : generators) {
        gen->update();
        sample += gen->getSample();
    }
    return std::clamp(sample, -1.0f, 1.0f);
}
void AudioContext::getAllSamples(float *buffer, int numFrames, int numChannels) {
    for (int i = 0; i < numFrames; ++i) {
        float sample = getAllSamples();  // mono
        for (int c = 0; c < numChannels; ++c) {
            buffer[i * numChannels + c] = sample;  // duplicate for stereo
        }
    }
}