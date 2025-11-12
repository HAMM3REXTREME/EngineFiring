#include "SoundBank.h"
SoundBank::SoundBank() {}

void SoundBank::addFromWavs(const std::vector<std::string> &filenames) {
    for (const std::string &file : filenames) {
        samples.push_back(AudioVector(file));
    }
}
SoundBank::SoundBank(const std::vector<std::string> &filenames){
    addFromWavs(filenames);
}
SoundBank::SoundBank(const std::string& sbFilename){
    addFromWavList(sbFilename);
}
void SoundBank::clearAll() { samples.clear(); }