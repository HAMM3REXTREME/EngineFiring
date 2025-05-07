#include <algorithm>
#include <portaudio.h>
#include <sndfile.h>

#include <string>
#include <vector>

#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SoundGenerator.h"
#include "TurboWhooshGenerator.h"
#include "SimpleSoundGenerator.h"

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
        *out++ = std::clamp(sample, -1.0f, 1.0f);
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
    float sample_rate = 44100;

    SoundBank mainSamples;
    mainSamples.loadFromWavs({"thump_library/note_64.wav", "thump_library/note_65.wav", "thump_library/note_66.wav", "thump_library/note_67.wav",
        "thump_library/note_68.wav", "thump_library/note_69.wav", "thump_library/note_70.wav", "thump_library/note_71.wav",
        "thump_library/note_72.wav", "thump_library/note_73.wav", "thump_library/note_74.wav", "thump_library/note_75.wav"});
    SoundBank turboSamples;
    turboSamples.loadFromWavs({"boom.wav","backfireEXT_4.wav","thump.wav", "flutter.wav"});


    // Engine engineDef("Countach V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 1);
    //Engine engineDef("Murcielago V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 6.25);
    //Engine engineDef("Audi V10 FSI", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 5);
    //Engine engineDef("1LR-GUE V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 5);
    //Engine engineDef("F1 V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 12.5);
    //Engine engineDef("Audi V8", Engine::getFiringOrderFromString("1-5-4-8-6-3-7-2"), 4);
    Engine engineDef("BMW N54", Engine::getFiringOrderFromString("1-5-3-6-2-4"),3);
    //Engine superchargerDef("Supercharger", {0},15);
    Engine turboshaftDef("Turbo Shaft - BorgWarner", {0},15);
    EngineSoundGenerator engine(mainSamples, engineDef,1000.0f, 0.4f);
    //EngineSoundGenerator supercharger(mainSamples, superchargerDef, 1000.0f, 0.1f);
    EngineSoundGenerator turboShaft(mainSamples, turboshaftDef, 1000.0f, 0.05f);
    TurboWhooshGenerator whoosh(sample_rate);
    SimpleSoundGenerator turboGen(turboSamples);
    BackfireSoundGenerator backfire(sample_rate);
    turboGen.setAmplitude(0.5);
    AudioContext context{.generators = {&engine, &whoosh, &turboShaft, &turboGen, &backfire}};
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
            turboGen.startPlayback(3);
            engine.setAmplitude(0.2f);
            backfire.setIntensity(5);
        }
        if (engine.getRPM() >= 4500){
            boost += 0.01;
        }
        if (engine.getRPM() <= 6000) {
            shifting = false;
            engine.setAmplitude(0.4f);
            backfire.setIntensity(0);
        }
        if (!shifting) {
            engine.setRPM(engine.getRPM() +  50000 / engine.getRPM());
            //supercharger.setRPM(engine.getRPM());
        } else {
            engine.setRPM(engine.getRPM() -  50000 / engine.getRPM());
            //supercharger.setRPM(engine.getRPM());
            boost = 0;
        }
        whoosh.setIntensity(boost/4);
        whoosh.setAmplitude(boost*0.01 + 0.2f);
        turboShaft.setRPM(10000 + boost*10000);
        std::cout << engine.getRPM() << " Boost: " << boost << "\n";
    }
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
