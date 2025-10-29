#include "AudioContext.h"
#include <sstream>

AudioContext::AudioContext(std::vector<SoundGenerator*> generators) : id("no name"), sound_generators(generators), m_ctx_amplitude(1.0f) {}
AudioContext::AudioContext(const std::string &new_id, std::vector<SoundGenerator*> generators) : id(new_id), sound_generators(generators), m_ctx_amplitude(1.0f) {}
AudioContext::AudioContext() : id("no name"), m_ctx_amplitude(1.0f) {}

void AudioContext::addGenerator(SoundGenerator *generator) { sound_generators.push_back(generator); }

float AudioContext::getAllSamples() {
    float sample = 0.0f;
    for (auto& gen : sound_generators) {
        gen->update();
        sample += gen->getSample() * m_ctx_amplitude;
    }
    sample = fx_chain.process(sample);
    return sample;
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

void AudioContext::setAmplitude(float amp) { m_ctx_amplitude = amp; }
float AudioContext::getAmplitude() const { return m_ctx_amplitude; }

std::string AudioContext::getInfo(int depth) const {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i) {
        oss << "    ";
    }
    if (depth == 0) {
        oss << "Root ";
    }
    oss << "AudioContext: id '" << id << "' amplitude " << m_ctx_amplitude << ", " << fx_chain.size() << " post filters, " << sound_generators.size()
        << " generators:\n";
    for (auto *gen : sound_generators) {
        oss << gen->getInfo(depth + 1);
    }
    if (depth == 0) {
        oss << "================\n";
    }
    return oss.str();
}
