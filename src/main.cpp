#include <portaudio.h>
#include <sndfile.h>

#include <string>
#include <vector>

#include "AudioVector.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"

int audio_callback(const void *, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *userData) {
    EngineSoundGenerator *engine = static_cast<EngineSoundGenerator *>(userData);
    float *out = static_cast<float *>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        engine->update();
        *out++ = engine->getSample();
    }

    return paContinue;
}

int main() {
    // std::vector<std::string> files = {
    //     "audi5/1_audi5cyl.wav",
    //     "audi5/2_audi5cyl.wav",
    //     "audi5/3_audi5cyl.wav",
    //     "audi5/4_audi5cyl.wav",
    //     "audi5/5_audi5cyl.wav"
    // };

    std::vector<std::string> files = {"thump_library/note_64.wav", "thump_library/note_65.wav", "thump_library/note_66.wav", "thump_library/note_67.wav",
                                      "thump_library/note_68.wav", "thump_library/note_69.wav", "thump_library/note_70.wav", "thump_library/note_71.wav",
                                      "thump_library/note_72.wav", "thump_library/note_73.wav", "thump_library/note_74.wav", "thump_library/note_75.wav"};
    std::vector<AudioVector> pistonSamples;
    for (const std::string &file : files) {
        pistonSamples.push_back(AudioVector(file));
    }

    // Engine engineDefCountach("V12", pistonSamples, {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 1);
    // Engine engineDefAudi("V10", pistonSamples, {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 1);
    Engine engineDefLFA("V10", pistonSamples, {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 1);
    EngineSoundGenerator engine(engineDefLFA, 1000.0f);

    int sample_rate = 44100;
    bool shifting = false;

    // Live audio
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sample_rate, 256, audio_callback, &engine);
    Pa_StartStream(stream);

    while (true) {
        Pa_Sleep(10);
        // Test sweep
        if (engine.getRPM() >= 40000) {
            shifting = true;
        }
        if (engine.getRPM() <= 30000) {
            shifting = false;
        }
        if (!shifting) {
            engine.setRPM(engine.getRPM() + 1000000 / engine.getRPM());
        } else {
            engine.setRPM(engine.getRPM() - 50000000 / engine.getRPM());
        }
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
