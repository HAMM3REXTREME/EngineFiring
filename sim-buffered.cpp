#include <portaudio.h>
#include <sndfile.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <csignal>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

constexpr int sample_rate = 44100;
constexpr int channels = 1;
constexpr float max_amplitude = 0.5f;  // Reduce to prevent clipping

//std::atomic<bool> running(true);

// Generate even firing intervals for a given number of cylinders
std::vector<float> evenFiring(int cylinders){
    std::vector<float> result(cylinders);

    for(int i=0; i<cylinders; i++){
        result[i] = 720.0f / cylinders;

    }
    return result;
}


// TODO: Move to AudioVector class constructor
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
            for (int ch = 0; ch < sfinfo.channels; ++ch) sum += samples[i * sfinfo.channels + ch];
            mono[i] = sum / sfinfo.channels;
        }
        return mono;
    }

    return samples;
}

class EngineSoundGenerator {
   public:

   // TODO: Move to Engine class
    void updateFiringIntervalFactors(std::vector<float> degrees) {
        // Must add up to 720 degrees or it won't sound right
        if (std::accumulate(degrees.begin(), degrees.end(), 0.0f) != 720) {
            std::cerr << "Firing order doesn't make any sense\n";
        }
        cylinderCount = firing_order.size();
        if (degrees.size() != cylinderCount) {
            std::cerr << "Firing order list doesn't match cylinder count\n";
        }
        // Calculate relative interval for each cylinder
        float evenFireInterval = 720.0f / cylinderCount;
        intervals_factor.clear();
        std::cout << "Firing factors (" << cylinderCount << " cylinders): ";
        for (float fireInterval : degrees) {
            intervals_factor.push_back(fireInterval / evenFireInterval);
            std::cout << "  " << fireInterval / evenFireInterval << "  ";
        }
        std::cout << "\n";
    }
    EngineSoundGenerator(const std::vector<std::string>& files, const std::vector<int>& firing_order, const std::vector<float>& degrees) : firing_order(firing_order), interval_timer(0.0f), rpm(3400.0f), phase(0) {
        for (const auto& file : files) {
            pistons.push_back(load_wav(file));
        }
        interval = 60.0f / rpm * sample_rate;
        updateFiringIntervalFactors(degrees); // TODO: Just get normalized factors from the Engine object's member
    }

    void setRPM(float newRPM) {
        rpm = newRPM;
        interval = 60.0f / rpm * sample_rate;
    }

    void update() {
        interval_timer += 1.0f;
        if (interval_timer >= interval) {
            // Schedule new piston fire
            int piston_index = firing_order[phase % cylinderCount];
            active_firings.push_back({&pistons[piston_index], 0});
            phase++;
            std::string firingVisual(cylinderCount, '-');
            firingVisual[piston_index] = (char)piston_index + '0';
            //std::cout << firingVisual << "\n";
            // std::cout << "Interval timer -= " << intervals_factor[piston_index] * interval << "\n";
            interval_timer -= intervals_factor[piston_index] * interval;  // Larger number = slower, 0.1 etc. = faster
        }
    }
    float getRPM() { return rpm; }
    float getSample() {
        float sample = 0.0f;
        // Mix active firings
        for (auto it = active_firings.begin(); it != active_firings.end();) {
            if (it->second < it->first->size()) {
                sample += (*(it->first))[it->second++] * max_amplitude;
                ++it;
            } else {
                it = active_firings.erase(it);
            }
        }
        return std::clamp(sample, -1.0f, 1.0f);
    }

   private:
    std::vector<std::vector<float>> pistons;
    int cylinderCount;
    std::vector<int> firing_order;
    std::vector<std::pair<const std::vector<float>*, size_t>> active_firings;
    float interval;                       // samples between firings
    std::vector<float> intervals_factor;  // Intervals (not rpm corrected, just factors) (should all be 1 for even fire)
    float interval_timer;
    float rpm;
    int phase;
};

// PortAudio stream callback
int audio_callback(const void*, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* userData) {
    EngineSoundGenerator* engine = static_cast<EngineSoundGenerator*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        engine->update();
        *out++ = engine->getSample();
    }

    return paContinue;
}

// Signal handler for clean exit
//void handle_sigint(int) { running = false; }

int main() {
    //signal(SIGINT, handle_sigint);

    // std::vector<std::string> files = {
    //     "audi5/1_audi5cyl.wav",
    //     "audi5/2_audi5cyl.wav",
    //     "audi5/3_audi5cyl.wav",
    //     "audi5/4_audi5cyl.wav",
    //     "audi5/5_audi5cyl.wav"
    // };

   std::vector<std::string> files = {"piano_keys/note_64.wav", "piano_keys/note_65.wav", "piano_keys/note_66.wav", "piano_keys/note_67.wav", "piano_keys/note_68.wav", "piano_keys/note_69.wav",
                                     "piano_keys/note_70.wav", "piano_keys/note_71.wav", "piano_keys/note_72.wav", "piano_keys/note_73.wav", "piano_keys/note_74.wav", "piano_keys/note_75.wav"};

    //EngineSoundGenerator engine(files, {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, evenFiring(12)); // Countach
    EngineSoundGenerator engine(files, {0,5,4,9,1,6,2,7,3,8}, evenFiring(10)); // LFA

    Pa_Initialize();
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sample_rate, 256, audio_callback, &engine);
    Pa_StartStream(stream);

    bool shifting = false;
    while (true) {
        Pa_Sleep(10);
        // Test sweep
        if (engine.getRPM() >= 35000) {
            shifting = true;
        }
        if (engine.getRPM() <= 20000) {
            shifting = false;
        }
        if (!shifting){
        engine.setRPM(engine.getRPM() + 400000 / engine.getRPM());
        }
        else{
            engine.setRPM(engine.getRPM() - 9000000 / engine.getRPM());
        }
    }

    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
