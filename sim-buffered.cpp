#include <sndfile.h>
#include <portaudio.h>
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <atomic>
#include <csignal>
#include <algorithm>
#include <numeric> 

constexpr int SAMPLE_RATE = 44100;
constexpr int CHANNELS = 1;
constexpr float MAX_AMPLITUDE = 0.5f;  // Reduce to prevent clipping

std::atomic<bool> running(true);

std::vector<float> load_wav(const std::string& filename) {
    SF_INFO sfinfo;
    SNDFILE* file = sf_open(filename.c_str(), SFM_READ, &sfinfo);
    if (!file) {
        std::cerr << "Failed to open " << filename << "\n";
        exit(1);
    }

    std::vector<float> samples(sfinfo.frames * sfinfo.channels);
    sf_readf_float(file, samples.data(), sfinfo.frames);
    sf_close(file);

    if (sfinfo.channels > 1) {
        std::vector<float> mono(sfinfo.frames);
        for (sf_count_t i = 0; i < sfinfo.frames; ++i) {
            float sum = 0.0f;
            for (int ch = 0; ch < sfinfo.channels; ++ch)
                sum += samples[i * sfinfo.channels + ch];
            mono[i] = sum / sfinfo.channels;
        }
        return mono;
    }

    return samples;
}

class EngineSoundGenerator {
public:
    void updateFiringIntervalFactors(std::vector<float> degrees){
        if (std::accumulate(degrees.begin(), degrees.end(), 0.0f) != 720){
            std::cerr << "Firing order doesn't make any sense\n"; 
        }
        int cylinderCount = firing_order.size();
        float evenFireInterval = 720.0f/cylinderCount;
        intervals_factor.clear();
        std::cout << "New firing factors: ";
        for (float fireInterval: degrees){
            intervals_factor.push_back(fireInterval/evenFireInterval);
            std::cout << "  " << fireInterval/evenFireInterval << "  ";
        }
        std::cout << "\n";

    }
    EngineSoundGenerator(const std::vector<std::string>& files)
        : firing_order{0, 1, 3, 4, 2}, interval_timer(0.0f), rpm(400.0f), phase(0) {
        for (const auto& file : files) {
            pistons.push_back(load_wav(file));
        }
        interval = 60.0f / rpm * SAMPLE_RATE;
        updateFiringIntervalFactors({144,144,144,144,144});
        
    }

    void setRPM(float newRPM) {
        rpm = newRPM;
        interval = 60.0f / rpm * SAMPLE_RATE;
    }

    void update() {
        interval_timer += 1.0f;
        if (interval_timer >= interval) {
            // Schedule new piston fire
            int piston_index = firing_order[phase % firing_order.size()];
            active_firings.push_back({&pistons[piston_index], 0});
            phase++;
            std::cout << "Interval timer -= " << intervals_factor[piston_index] * interval << "\n";
            interval_timer -= intervals_factor[piston_index] * interval; // Larger number = slower, 0.1 etc. = faster
        }
    }
float getRPM(){
return rpm;
}
    float getSample() {
        float sample = 0.0f;
        // Mix active firings
        for (auto it = active_firings.begin(); it != active_firings.end();) {
            if (it->second < it->first->size()) {
                sample += (*(it->first))[it->second++] * MAX_AMPLITUDE;
                ++it;
            } else {
                it = active_firings.erase(it);
            }
        }
        return std::clamp(sample, -1.0f, 1.0f);
    }

private:
    std::vector<std::vector<float>> pistons;
    std::vector<int> firing_order;
    std::vector<std::pair<const std::vector<float>*, size_t>> active_firings;
    float interval;      // samples between firings
    std::vector<float> intervals_factor; // Intervals (not rpm corrected, just factors) (should all be 1 for even fire)
    float interval_timer;
    float rpm;
    int phase;
};

// PortAudio stream callback
int audio_callback(const void*, void* outputBuffer,
                   unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo*,
                   PaStreamCallbackFlags, void* userData) {
    EngineSoundGenerator* engine = static_cast<EngineSoundGenerator*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        engine->update();
        *out++ = engine->getSample();
    }

    return paContinue;
}

// Signal handler for clean exit
void handle_sigint(int) {
    running = false;
}

int main() {
    signal(SIGINT, handle_sigint);

    std::vector<std::string> files = {
        "1_ V12 kicker.wav", "2_ V12 kicker.wav", "3_ V12 kicker.wav",
        "4_ V12 kicker.wav", "5_ V12 kicker.wav"
    };

    EngineSoundGenerator engine(files);

    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256,
                         audio_callback, &engine);
    Pa_StartStream(stream);

    std::cout << "Streaming audio. Press Ctrl+C to quit.\n";
    while (running.load()) {
        Pa_Sleep(10);
        // Test sweep
        if(engine.getRPM()>= 17500){
            engine.setRPM(14000.0f);
        }
        engine.setRPM(engine.getRPM()+100000/engine.getRPM());
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
