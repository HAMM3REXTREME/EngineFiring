
#pragma once

#include "AudioVector.h"
class SoundBank{
    public:
        std::vector<AudioVector> samples;
        void loadFromWavs(const std::vector<std::string>& filenames){
            for (const std::string &file : filenames) {
                samples.push_back(AudioVector(file));
            }
        
        }

};