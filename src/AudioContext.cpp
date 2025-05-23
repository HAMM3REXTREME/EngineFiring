#include "AudioContext.h"
#include "Biquad.h"

AudioContext::AudioContext(std::vector<SoundGenerator *> generators) : generators(generators) {}
AudioContext::AudioContext() {}

void AudioContext::addGenerator(SoundGenerator *generator) { generators.push_back(generator); }

float AudioContext::getAllSamples() {
    float sample = 0.0f;
    for (auto *gen : generators) {
        gen->update();
        sample += gen->getSample();
    }
    sample = lowShelfFilter.process(sample);
    sample = midBoostFilter.process(sample);
    sample = highShelfFilter.process(sample);
    return std::clamp(sample, -1.0f, 1.0f);
}
void AudioContext::getAllSamples(float *buffer, int numFrames, int numChannels) {
    for (int i = 0; i < numFrames; ++i) {
        float sample = getAllSamples(); // mono
        for (int c = 0; c < numChannels; ++c) {
            buffer[i * numChannels + c] = sample; // duplicate for stereo
        }
    }
}

void AudioContext::configureEQ(float sampleRate) {
    lowShelfFilter.setBiquad(bq_type_lowshelf, 150.0f / sampleRate, 0.707f, -12.0f); // cut lows
    midBoostFilter.setBiquad(bq_type_peak, 1500.0f / sampleRate, 1.0f, 10.0f);       // boost mids
    highShelfFilter.setBiquad(bq_type_highshelf, 8000.0f / sampleRate, 0.707f, 4.0f); // boost highs
}