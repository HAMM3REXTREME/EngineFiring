#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <portaudio.h>
#include <sndfile.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Car.h"
#include "Damper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SimpleSoundGenerator.h"
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
        *out++ = std::clamp(sample, -1.0f, 1.0f);
    }

    return paContinue;
}

void manageCar(Car *car, std::atomic<bool> *run) {
    while (*run) {
        car->tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
int main() {
    // std::vector<std::string> files = {
    //     "audi5/1_audi5cyl.wav",
    //     "audi5/2_audi5cyl.wav",
    //     "audi5/3_audi5cyl.wav",
    //     "audi5/4_audi5cyl.wav",
    //     "audi5/5_audi5cyl.wav"
    // };
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Engine Firing Simulator");
    float sample_rate = 44100;

    SoundBank mainSamples;
    mainSamples.loadFromWavs({"thump_library/note_64.wav", "thump_library/note_65.wav", "thump_library/note_66.wav", "thump_library/note_67.wav",
                              "thump_library/note_68.wav", "thump_library/note_69.wav", "thump_library/note_70.wav", "thump_library/note_71.wav",
                              "thump_library/note_72.wav", "thump_library/note_73.wav", "thump_library/note_74.wav", "thump_library/note_75.wav"});
    SoundBank turboSamples;
    turboSamples.loadFromWavs({"boom.wav", "backfireEXT_4.wav", "thump.wav", "flutter.wav"});

    // Engine engineDef("Countach V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 4.5);
    // Engine engineDef("Murcielago V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 7);
    // Engine engineDef("Audi V10 FSI", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 5);
    // Engine engineDef("1LR-GUE V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 5);
    // Engine engineDef("F1 V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 12.5);
    // Engine engineDef("Audi V8", Engine::getFiringOrderFromString("1-5-4-8-6-3-7-2"), 4);
    Engine engineDef("BMW N54", Engine::getFiringOrderFromString("1-5-3-6-2-4"), 3);
    // Engine engineDef("Audi i5", Engine::getFiringOrderFromString("1-2-4-5-3"),3);
    // Engine engineDef("4 Banger", Engine::getFiringOrderFromString("1-3-4-2"),2);
    // Engine superchargerDef("Supercharger", {0},15);
    Engine turboshaftDef("Turbo Shaft - BorgWarner", {0}, 15);
    EngineSoundGenerator engine(mainSamples, engineDef, 1000.0f, 0.5f);
    // EngineSoundGenerator supercharger(mainSamples, superchargerDef, 1000.0f, 0.1f);
    EngineSoundGenerator turboShaft(mainSamples, turboshaftDef, 1000.0f, 0.05f);
    TurboWhooshGenerator whoosh(sample_rate);
    SimpleSoundGenerator turboGen(turboSamples);
    BackfireSoundGenerator backfire(sample_rate);
    turboGen.setAmplitude(0.01f);
    AudioContext context{.generators = {&engine, &turboGen, &turboShaft, &backfire, &whoosh}};
    // Sample sweep
    bool shifting = false;

    Car car;
    std::atomic<bool> carRunning = true;
    std::thread carThread{manageCar, &car, &carRunning};
    car.ignition = true;
    car.setRPM(800);

    // Live audio
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, sample_rate, 256, audio_callback, &context);
    Pa_StartStream(stream);
    bool blowoff = false;
    float boost = 0;
    float gas = 0;
    int frame = 0;
    int lastGear = 1;
    int upShiftFrame = 0;
int downShiftFrame = 0;
    int lastLiftOff = 0;
    Damper gasAvg(5);
    backfire.setAmplitude(0.3f);
    while (window.isOpen()) {
        frame++;
        Pa_Sleep(10);
        // Process events
        while (const std::optional event = window.pollEvent()) {
            // Close window: exit
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::T) {
                    gas = 125;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Up) {
                    car.setGear(car.getGear() + 1);
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Down) {
                    car.setGear(car.getGear() - 1);
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 10;
                }
            } else if (const auto *keyPressed = event->getIf<sf::Event::KeyReleased>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::T) {
                    gas = 0;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 0;
                }
            }
                if (const auto *joystickMove = event->getIf<sf::Event::JoystickMoved>()){
                if (joystickMove->axis == sf::Joystick::Axis::Z){
                    //std:: cout << "Moved: " << joystickMove->position << "\n";
                    gas = 0.75*(100 - joystickMove->position);
                }
            }
            else if (const auto* joystickButton = event->getIf<sf::Event::JoystickButtonPressed>()) {
                // Handle button presses
                if (joystickButton->button == 4) {  // LB button
                    std::cout << "Upshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas(0);
                    upShiftFrame = frame + 20;
                }
                else if (joystickButton->button == 5) {  // RB button
                    std::cout << "Downshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas(150);
                    downShiftFrame = frame + 30;
                }
            }
        }
        if (upShiftFrame == 0 && downShiftFrame == 0) {
            gasAvg.addValue(gas);
            car.setGas(gasAvg.getAverage());
        }
        
        if (frame == upShiftFrame) {
            car.setGear(lastGear+1);
            upShiftFrame = 0;
        }
        
        if (frame == downShiftFrame) {
            car.setGear(lastGear-1);
            downShiftFrame = 0;
        }

        if (car.getGas() <= 10 && (lastLiftOff + 100 >= frame)) {
            backfire.setIntensity(1.0f-((frame-lastLiftOff)/100.0f));
        }
        if (car.getRPM() <= 4000 || car.getGas() >= 25){
            backfire.setIntensity(0);
        }
        if (car.getBoost() <= 15 && blowoff == false) {
            blowoff = true;
            turboGen.startPlayback(3);
            lastLiftOff = frame;
        }
        if (car.getBoost() >= 50) {
            blowoff = false;
        }

        engine.setRPM(car.getRPM());
        engine.setAmplitude(car.getTorque() / 500 + 0.2f);
        whoosh.setIntensity(car.getBoost() / 150);
        whoosh.setAmplitude(car.getBoost() / 2500);
        turboShaft.setAmplitude(car.getBoost() / 750);
        turboShaft.setRPM(10000 + car.getBoost() * 100);

        window.clear();
        //std::cout << "RPM: " << (int)engine.getRPM() << "  boost: " << car.getBoost() << " Gear:" << car.getGear() << "\n";
        window.display();
    }
    carRunning = false;
    carThread.join();
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
