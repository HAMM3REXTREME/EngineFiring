#include "AudioVector.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <iostream>

// Load from WAV (converts stereo/multichannel to mono)
int AudioVector::loadfromWav(const std::string &filename) {
    unsigned int channels;
    unsigned int sampleRate;
    drwav_uint64 totalFrameCount;

    float *pSampleData = drwav_open_file_and_read_pcm_frames_f32(filename.c_str(), &channels, &sampleRate, &totalFrameCount, nullptr);

    if (pSampleData == nullptr) {
        std::cerr << "Failed to load WAV: " << filename << std::endl;
        return 1;
    }

    samples.resize(totalFrameCount);

    // Downmix to mono if needed
    if (channels == 1) {
        std::copy(pSampleData, pSampleData + totalFrameCount, samples.begin());
    } else {
        for (drwav_uint64 i = 0; i < totalFrameCount; ++i) {
            float sum = 0.0f;
            for (unsigned int ch = 0; ch < channels; ++ch) {
                sum += pSampleData[i * channels + ch];
            }
            samples[i] = sum / channels;
        }
    }

    drwav_free(pSampleData, nullptr);
    return 0;
}

// Save to mono WAV
int AudioVector::saveToWav(const std::string &filename) {
    drwav_data_format format;
    format.container = drwav_container_riff;
    format.format = DR_WAVE_FORMAT_IEEE_FLOAT;
    format.channels = 1;
    format.sampleRate = 48000;
    format.bitsPerSample = 32;

    drwav wav;
    if (!drwav_init_file_write(&wav, filename.c_str(), &format, nullptr)) {
        std::cerr << "Failed to write WAV: " << filename << std::endl;
        return 1;
    }

    drwav_uint64 written = drwav_write_pcm_frames(&wav, samples.size(), samples.data());
    drwav_uninit(&wav);

    return written == samples.size() ? 0 : 1;
}

// Constructors
AudioVector::AudioVector() {}
AudioVector::AudioVector(const std::string &filename) { loadfromWav(filename); }
AudioVector::AudioVector(const std::vector<float> &m_samples) { samples = m_samples; }
