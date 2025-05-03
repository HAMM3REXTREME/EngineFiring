#include <portaudio.h>
#include <sndfile.h>

#include <iostream>
#include <string>
#include <vector>

#include "AudioVector.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"

int audio_callback(const void*, void* outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* userData) {
    EngineSoundGenerator* engine = static_cast<EngineSoundGenerator*>(userData);
    float* out = static_cast<float*>(outputBuffer);

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

    std::vector<std::string> files = {"piano_keys/note_64.wav", "piano_keys/note_65.wav", "piano_keys/note_66.wav", "piano_keys/note_67.wav", "piano_keys/note_68.wav", "piano_keys/note_69.wav",
                                      "piano_keys/note_70.wav", "piano_keys/note_71.wav", "piano_keys/note_72.wav", "piano_keys/note_73.wav", "piano_keys/note_74.wav", "piano_keys/note_75.wav"};
    std::vector<AudioVector> pistonSamples;
    for (const std::string& file : files) {
        pistonSamples.push_back(AudioVector(file));
    }
    Engine engineDef("V12", pistonSamples, {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 1);
    Engine engineDefGallardo("V10", pistonSamples, {0, 5, 4, 9, 1, 6, 2, 7, 3, 8},{90,54,90,54,90,54,90,54,90,54}, 1);

    EngineSoundGenerator engine(engineDefGallardo, 1000.0f);
    EngineSoundGenerator engineLambo(engineDefGallardo, 1000.0f);
    // EngineSoundGenerator engine(1.4,pistonSamples, {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, evenFiring(12));  // Countach
    // EngineSoundGenerator engineLFA(1, pistonSamples, {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, evenFiring(10));       // LFA
    // EngineSoundGenerator engine(1.4, pistonSamples, {0,5,4,9,1,6,2,7,3,8}, {90,54,90,54,90,54,90,54,90,54}); // Audi R8
    // EngineSoundGenerator engine(0.35,pistonSamples, {0,2,3,1}); // 4 Banger
    // EngineSoundGenerator engine(pistonSamples, {0,4,3,7,2,6,1,5}, evenFiring(8)); // 5.2L Voodoo

    int sample_rate = 44100;
    bool shifting = false;

    // Dump to file
    AudioVector debugRecord = AudioVector();
    while (debugRecord.samples.size() <= 175 * sample_rate) {
        engineLambo.update();
        if (engineLambo.getRPM() >= 35000) {
            shifting = true;
        }
        if (engineLambo.getRPM() <= 25000) {
            shifting = false;
        }
        if (!shifting) {
            engineLambo.setRPM(engineLambo.getRPM() + 900 / engineLambo.getRPM());
        } else {
            engineLambo.setRPM(engineLambo.getRPM() - 10000 / engineLambo.getRPM());
        }
        debugRecord.samples.push_back(engineLambo.getSample());
    }
    debugRecord.saveToWav("debug.wav");
    std::cout << "Saved to debug.wav\n";

    // Live audio
    Pa_Initialize();
    PaStream* stream;
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
