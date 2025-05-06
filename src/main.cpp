#include <algorithm>
#include <portaudio.h>
#include <sndfile.h>

#include <string>
#include <vector>

#include "AudioVector.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SoundGenerator.h"
#include "TurboWhooshGenerator.h"

struct AudioContext {
    std::vector<SoundGenerator *> generators;
};

int audio_callback(const void *, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *userData) {
    auto *ctx = static_cast<AudioContext *>(userData);
    float *out = static_cast<float *>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        float sample = 0.0f;
        for (auto *gen : ctx->generators) {
            gen->update();
            sample += gen->getSample();
        }
        *out++ = std::clamp(sample/ctx->generators.size(), -1.0f, 1.0f);
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
    int sample_rate = 44100;
    std::vector<std::string> files = {"thump_library/note_64.wav", "thump_library/note_65.wav", "thump_library/note_66.wav", "thump_library/note_67.wav",
                                      "thump_library/note_68.wav", "thump_library/note_69.wav", "thump_library/note_70.wav", "thump_library/note_71.wav",
                                      "thump_library/note_72.wav", "thump_library/note_73.wav", "thump_library/note_74.wav", "thump_library/note_75.wav"};
    SoundBank mainSamples;
    mainSamples.loadFromWavs(files);
    // Engine engineDef("Countach V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 1);
    Engine engineDef("Murcielago V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 6.25);
    //Engine engineDef("Audi V10 FSI", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 5);
    //Engine engineDef("1LR-GUE V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 5);
    //Engine engineDef("F1 V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 12.5);
    //Engine engineDef("Audi V8", Engine::getFiringOrderFromString("1-5-4-8-6-3-7-2"), 4);
    //Engine engineDef("BMW N54", Engine::getFiringOrderFromString("1-5-3-6-2-4"),3);
    Engine superchargerDef("Supercharger", {0},15);
    Engine turboshaftDef("Turbo Shaft - BorgWarner", {0},10);
    EngineSoundGenerator engine(mainSamples, engineDef,1000.0f, 0.5f);
    EngineSoundGenerator supercharger(mainSamples, superchargerDef, 1000.0f, 0.3f);
    EngineSoundGenerator turboShaft(mainSamples, turboshaftDef, 1000.0f, 0.3f);
    TurboWhooshGenerator whoosh(sample_rate);
    AudioContext context{.generators = {&engine, &whoosh, &turboShaft}};
    // Sample sweep
    bool shifting = false;

    // Live audio
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sample_rate, 256, audio_callback, &context);
    Pa_StartStream(stream);
    float boost = 0;
    while (true) {
        Pa_Sleep(10);
        // Test sweep
        if (engine.getRPM() >= 8000) {
            shifting = true;
        }
        if (engine.getRPM() >= 6500){
            boost += 0.01;
        }
        if (engine.getRPM() <= 6000) {
            shifting = false;
            boost = 0;
        }
        if (!shifting) {
            engine.setRPM(engine.getRPM() +  50000 / engine.getRPM());
            supercharger.setRPM(engine.getRPM());
        } else {
            engine.setRPM(engine.getRPM() -  5000000 / engine.getRPM());
            supercharger.setRPM(engine.getRPM());
        }
        whoosh.setIntensity(boost/4);
        whoosh.setGain(boost*0.2 + 0.2f);
        turboShaft.setRPM(boost*10000 + 1000);
        std::cout << engine.getRPM() << " Boost: " << boost << "\n";
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
