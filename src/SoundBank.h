
#pragma once

#include "AudioVector.h"
class SoundBank {
  public:
    SoundBank();
    SoundBank(const std::vector<std::string> &filenames);
    std::vector<AudioVector> samples;
    void addFromWavs(const std::vector<std::string> &filenames);
    void clearAll();
};