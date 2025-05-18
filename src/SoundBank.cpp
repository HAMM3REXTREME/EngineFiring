#include "SoundBank.h"
SoundBank::SoundBank() {}

void SoundBank::addFromWavs(const std::vector<std::string> &filenames) {
    for (const std::string &file : filenames) {
        samples.push_back(AudioVector(file));
    }
}
void SoundBank::clearAll() { samples.clear(); }