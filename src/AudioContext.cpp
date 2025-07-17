#include "AudioContext.h"
#include "Biquad.h"
#include <sstream>

AudioContext::AudioContext(std::vector<SoundGenerator *> generators) : id("no name"), generators(generators), ctxAmplitude(1.0f) {}
AudioContext::AudioContext(const std::string &new_id, std::vector<SoundGenerator *> generators) : id(new_id), generators(generators), ctxAmplitude(1.0f) {}
AudioContext::AudioContext() : id("no name"), ctxAmplitude(1.0f) {}

void AudioContext::addGenerator(SoundGenerator *generator) { generators.push_back(generator); }

float AudioContext::getAllSamples() {
    float sample = 0.0f;
    for (auto *gen : generators) {
        gen->update();
        sample += gen->getSample() * ctxAmplitude;
    }
    sample = fx.process(sample);
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

float AudioContext::getSample() { return getAllSamples(); }

void AudioContext::setAmplitude(float amp) { ctxAmplitude = amp; }
float AudioContext::getAmplitude() const { return ctxAmplitude; }

std::string AudioContext::getInfo(int depth) const {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i) {
        oss << "    ";
    }
    if (depth == 0) {
        oss << "Root ";
    }
    oss << "AudioContext: id '" << id << "' amplitude " << ctxAmplitude << ", " << fx.biquads.size() << " biquad filters, " << generators.size()
        << " generators:\n";
    for (auto *gen : generators) {
        oss << gen->getInfo(depth + 1);
    }
    if (depth == 0) {
        oss << "================\n";
    }
    return oss.str();
}
