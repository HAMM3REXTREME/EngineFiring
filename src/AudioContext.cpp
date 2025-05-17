#include "AudioContext.h"

AudioContext::AudioContext(std::vector<SoundGenerator *> generators) : generators(generators) {}

float AudioContext::getAllSamples(){
    float sample = 0.0f;
    for (auto *gen : generators) {
        gen->update();
        sample += gen->getSample();
    }
    return std::clamp(sample, -1.0f, 1.0f);
}