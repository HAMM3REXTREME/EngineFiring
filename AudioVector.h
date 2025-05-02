#ifndef AUDIOVECTOR_H
#define AUDIOVECTOR_H

#include <vector>
#include <string>

class AudioVector {
public:
    std::vector<float> samples;

    // Load audio from a file (converts to mono if needed)
    AudioVector(const std::string& filename);
    AudioVector();
    int loadfromWav(const std::string& filename);

    // Construct directly from sample vector
    AudioVector(const std::vector<float>& m_samples);

    // Save audio to mono .wav file
    int saveToWav(const std::string& filename);
};

#endif // AUDIOVECTOR_H
