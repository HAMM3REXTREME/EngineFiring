#include "AudioVector.h"

#include <sndfile.h>

#include <iostream>
#include <string>
#include <vector>
// Stores an audio file as a large vector of floats.
int AudioVector::loadfromWav(const std::string& filename) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        exit(1);
    }

    std::vector<float> fileSamples(sfinfo.frames * sfinfo.channels);
    sf_readf_float(file, fileSamples.data(), sfinfo.frames);
    sf_close(file);

    if (sfinfo.channels > 1) {
        std::vector<float> mono(sfinfo.frames);
        for (sf_count_t i = 0; i < sfinfo.frames; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < sfinfo.channels; ++ch) sum += fileSamples[i * sfinfo.channels + ch];
            mono[i] = sum / sfinfo.channels;
        }
        samples = mono;
        return 0;
    }

    samples = fileSamples;
    return 0;
}

int AudioVector::saveToWav(const std::string& filename) {
    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.samplerate = 44100;  // default to CD quality
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* file = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return 1;
    }

    sf_writef_float(file, samples.data(), samples.size());
    sf_close(file);
    return 0;
}
AudioVector::AudioVector() {}
AudioVector::AudioVector(const std::string& filename) { loadfromWav(filename); };
AudioVector::AudioVector(const std::vector<float>& m_samples) { samples = m_samples; };
