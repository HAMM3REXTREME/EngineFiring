
#pragma once

#include "AudioVector.h"
class SoundBank {
  public:
    SoundBank();
    std::vector<AudioVector> samples;
    void addFromWavs(const std::vector<std::string> &filenames);
    void addFromWav(const std::string& filename);
    void clearAll();
};