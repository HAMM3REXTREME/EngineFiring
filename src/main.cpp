#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <map>
#include <portaudio.h>
#include <sndfile.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "AudioContext.h"
#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Car.h"
#include "Damper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SimpleSoundGenerator.h"
#include "SoundBank.h"
#include "TurboWhooshGenerator.h"

constexpr float SAMPLE_RATE = 48000;
constexpr int WINDOW_X = 1080;
constexpr int WINDOW_Y = 720;

constexpr int DOWNSHIFT_DELAY = 250;
constexpr int UPSHIFT_DELAY = 190;


// Function for the PortAudio audio callback
int audio_callback(const void *, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *userData) {
    auto *audioContext = static_cast<AudioContext *>(userData);
    float *out = static_cast<float *>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        *out++ = audioContext->getAllSamples();
    }

    return paContinue;
}

// Run our car sim on a seperate thread
void manageCar(Car *car, std::atomic<bool> *run) {
    while (*run) {
        car->tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// Car starter sequence on a seperate thread for now
void carStarter(Car *car, bool *m_isStarting) {
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    std::cout << "Vroom!\n";
    car->setRPM(800);
    for (int i = 0; i < 10; i++) {
        car->setGas(17 * i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    car->setGas(0);
    *m_isStarting = false;
}

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_X, WINDOW_Y}), "Engine Firing Simulator");
    sf::Clock deltaClock;

    // ==== THE ENGINE
    SoundBank mainSamples;
    mainSamples.addFromWavs({"assets/audio/tick_library/note_64.wav",  "assets/audio/tick_library/note_65.wav",  "assets/audio/tick_library/note_66.wav",
                             "assets/audio/tick_library/note_67.wav",  "assets/audio/tick_library/note_68.wav",  "assets/audio/tick_library/note_69.wav",
                             "assets/audio/tick_library/note_70.wav",  "assets/audio/tick_library/note_71.wav",  "assets/audio/tick_library/note_72.wav",
                             "assets/audio/tick_library/note_73.wav",  "assets/audio/tick_library/note_74.wav",  "assets/audio/tick_library/note_75.wav",
                             "assets/audio/tick_library/note_76.wav",  "assets/audio/tick_library/note_77.wav",  "assets/audio/tick_library/note_78.wav",
                             "assets/audio/tick_library/note_79.wav",  "assets/audio/tick_library/note_80.wav",  "assets/audio/tick_library/note_81.wav",
                             "assets/audio/tick_library/note_82.wav",  "assets/audio/tick_library/note_83.wav",  "assets/audio/tick_library/note_84.wav",
                             "assets/audio/tick_library/note_85.wav",  "assets/audio/tick_library/note_86.wav",  "assets/audio/tick_library/note_87.wav",
                             "assets/audio/tick_library/note_88.wav",  "assets/audio/tick_library/note_89.wav",  "assets/audio/tick_library/note_90.wav",
                             "assets/audio/tick_library/note_91.wav",  "assets/audio/tick_library/note_92.wav",  "assets/audio/tick_library/note_93.wav",
                             "assets/audio/tick_library/note_94.wav",  "assets/audio/tick_library/note_95.wav",  "assets/audio/tick_library/note_96.wav",
                             "assets/audio/tick_library/note_97.wav",  "assets/audio/tick_library/note_98.wav",  "assets/audio/tick_library/note_99.wav",
                             "assets/audio/tick_library/note_100.wav", "assets/audio/tick_library/note_101.wav", "assets/audio/tick_library/note_102.wav"});

    Engine engineDef("Revuelto V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 6.5);
    // Engine engineDef("Diablo/Murci V12", Engine::getFiringOrderFromString("1-7-4-10-2-8-6-12-3-9-5-11"), 6);
    // Engine engineDef("Countach V12", Engine::getFiringOrderFromString("1 7 5 11 3 9 6 12 2 8 4 10"), 4.5);
    // Engine engineDef("F1 V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 16);
    // Engine engineDef("Audi V10 FSI", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 5.4);
    // Engine engineDef("1LR-GUE V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 5);
    // Engine engineDef("F1 V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 12.5);
    // Engine engineDef("Bugatti W16", Engine::getFiringOrderFromString("1 14 9 4 7 12 15 6 13 8 3 16 11 2 5 10"), 8.2);
    // Engine engineDef("Inline 9 - Experimental", Engine::getFiringOrderFromString("1 2 4 6 8 9 7 5 3"), 5);
    // Engine engineDef("Flat plane V8", Engine::getFiringOrderFromString("1 5 3 7 4 8 2 6"), 4);
    // Engine engineDef("Mercedes M120 V12", Engine::getFiringOrderFromString("1-12-5-8-3-10-6-7-2-11-4-9"),7.6);
    // Engine engineDef("Murican V8 +", Engine::getFiringOrderFromString("1-8-7-2-6-5-4-3"),3);
    // Engine engineDef("2UR-GSE V8", Engine::getFiringOrderFromString("1-8-7-3-6-5-4-2"),3);
    // Engine engineDef("BMW N54", Engine::getFiringOrderFromString("1-5-3-6-2-4"), 3);
    // Engine engineDef("VR6", Engine::getFiringOrderFromString("1-5-3-6-2-4"), {120, 130, 110, 125, 115, 120}, 3);
    // Engine engineDef("Audi i5", Engine::getFiringOrderFromString("1-2-4-5-3"),3);
    // Engine engineDef("Perfect Fifth i5", Engine::getFiringOrderFromString("1 3 5 2 4"), {120, 180, 120, 180, 120},3);
    // Engine engineDef("4 Banger", Engine::getFiringOrderFromString("1-3-4-2"),2);
    // Engine engineDef("Super Sport", Engine::getFiringOrderFromString("1-3-4-2"),4);
    // Engine engineDef("Nissan VQ", Engine::getFiringOrderFromString("1-2-3-4-5-6"),3);
    // Engine engineDef("Ford 4.0L V6", Engine::getFiringOrderFromString("1-4-2-5-3-6"),3);
    // Engine engineDef("Buick odd firing V6", Engine::getFiringOrderFromString("1-6-5-4-3-2"), {90,150,90,150,90,150},3);
    // Engine engineDef("Porsche Flat 6", Engine::getFiringOrderFromString("1-6-2-4-3-5"), 3.6);
    EngineSoundGenerator engine(mainSamples, engineDef, 1000.0f, 0.5f);
    EngineSoundGenerator engineAlt(mainSamples, engineDef, 1000.0f, 0.5f);
    EngineSoundGenerator engineAltAlt(mainSamples, engineDef, 1000.0f, 0.5f);
    engineAlt.setNoteOffset(22);
    engineAltAlt.setNoteOffset(16);

    // ==== SUPERCHARGER (Just a high revving 1 cylinder)
    Engine superchargerDef("Supercharger", {0}, 8);
    EngineSoundGenerator supercharger(mainSamples, superchargerDef, 1000.0f, 0.7f);
    supercharger.setAmplitude(0.5f);
    supercharger.setNoteOffset(20);

    // ==== GENERAL SOUND SAMPLES
    SoundBank generalSamples;
    generalSamples.addFromWavs({"assets/audio/extra/boom.wav", "assets/audio/extra/starter.wav"});
    SimpleSoundGenerator generalGen(generalSamples);
    generalGen.setAmplitude(0.3f);

    // ==== TURBOCHARGER SHAFT (Sounds like a faint supercharger)
    Engine turboshaftDef("BorgWarner K04 - Shaft", {0}, 8);
    EngineSoundGenerator turboShaft(mainSamples, turboshaftDef, 1000.0f, 0.05f);

    // ==== TURBO WHOOSH NOISE GENERATOR
    TurboWhooshGenerator whoosh(SAMPLE_RATE);

    // ==== TURBO FLUTTER AND BOV SOUNDS ETC.
    SoundBank turboSamples;
    turboSamples.addFromWavs({"assets/audio/extra/thump.wav", "assets/audio/extra/flutter.wav"});
    SimpleSoundGenerator turboGen(turboSamples);
    turboGen.setAmplitude(0.01f);

    // ==== BACKFIRE NOISE GENERATOR
    BackfireSoundGenerator backfire(SAMPLE_RATE);
    backfire.setAmplitude(0.2f);

    // Audio sample generators that get summed up and played together
    AudioContext engineCtx({&engine, &engineAlt, &engineAltAlt});
    AudioContext context({&whoosh, &backfire, &turboShaft, &turboGen, &generalGen, &engineCtx});
    // PortAudio for live audio playback
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256, audio_callback, &context);
    Pa_StartStream(stream);

    Car car;
    std::atomic<bool> carRunning = true;
    std::thread carThread{manageCar, &car, &carRunning};
    car.ignition = false;

    // Testing shift behavior etc.
    bool shifting = false;
    bool isStarting = false;
    bool blowoff = false;
    float boost = 0;
    float gas = 0;
    int frame = 0;    // Frame counter since program started
    int lastGear = 0; // Last (non-fake-clutch) gear
    int upShiftFrame = 0;
    int downShiftFrame = 0;
    int lastLiftOff = 0;
    bool shiftLock = false; // Only allow one type of shift at a time (upshift/downshift)
    Damper gasAvg(5);       // Smooth out gas inputs

    // Map user keyboard input to differen levels of throttle
    std::map<sf::Keyboard::Scancode, int> userThrottleMap;
    userThrottleMap[sf::Keyboard::Scancode::Q] = 30;
    userThrottleMap[sf::Keyboard::Scancode::W] = 50;
    userThrottleMap[sf::Keyboard::Scancode::E] = 80;
    userThrottleMap[sf::Keyboard::Scancode::R] = 130;
    userThrottleMap[sf::Keyboard::Scancode::T] = 150;

    // Tachometer needle
    sf::RectangleShape tach(sf::Vector2f(250.f, 6.f));  // Size of the tach
    tach.setFillColor(sf::Color::Red);                  // Color of the tach
    tach.setPosition({WINDOW_X / 2.f, WINDOW_Y / 2.f}); // Position of the tach
    tach.setOrigin({250.f, 3.f});                       // Center of rotation
    // Tachometer background
    sf::Texture textureTach;
    if (!textureTach.loadFromFile("assets/textures/dial.png", false, sf::IntRect({0, 0}, {1000, 1000}))) {
        std::cout << "Can't load texture\n";
        return -1;
    }
    textureTach.setSmooth(true);
    sf::Sprite spriteTach(textureTach);
    spriteTach.setOrigin({500, 500});
    spriteTach.setPosition({WINDOW_X / 2.f, WINDOW_Y / 2.f});
    spriteTach.setScale({0.6, 0.6});
    // Middle part thing
    sf::Texture textureMiddle;
    if (!textureMiddle.loadFromFile("assets/textures/middle.png", false, sf::IntRect({0, 0}, {1000, 1000}))) {
        std::cout << "Can't load texture\n";
        return -1;
    }
    textureMiddle.setSmooth(true);
    sf::Sprite spriteMiddle(textureMiddle);
    spriteMiddle.setOrigin({500, 500});
    spriteMiddle.setPosition({WINDOW_X / 2.f, WINDOW_Y / 2.f});
    spriteMiddle.setScale({0.07, 0.07});

        // Text Information
    sf::Font font;
    if (!font.openFromFile("assets/fonts/Aldrich-Regular.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return EXIT_FAILURE;
    }
    sf::Text gaugeValue(font, "Text", 24);
    gaugeValue.setFillColor(sf::Color::White);
    gaugeValue.setPosition({WINDOW_X / 2.f + 25.f, WINDOW_Y / 2.f + 25.f});

    // Biquad filters
    Biquad lowShelfFilter(bq_type_lowshelf, 150.0f / SAMPLE_RATE, 0.707f, -5.0f);  // cut lows
    Biquad midBoostFilter(bq_type_peak, 1500.0f / SAMPLE_RATE, 1.0f, 10.0f);        // boost mids
    Biquad highShelfFilter(bq_type_highshelf, 8000.0f / SAMPLE_RATE, 0.707f, 3.0f); // boost highs
    Biquad rumbleBoostFilter(bq_type_peak, 80.0f / SAMPLE_RATE, 0.5f, 9.0f);
    engineCtx.fx.addFilter(lowShelfFilter);
    engineCtx.fx.addFilter(midBoostFilter);
    engineCtx.fx.addFilter(highShelfFilter);
    engineCtx.fx.addFilter(rumbleBoostFilter);

    // ==== APPLICATION MAIN LOOP ====
    while (window.isOpen()) {
        frame++;
        // Time since last frame
        float deltaTime = deltaClock.restart().asMilliseconds();
        Pa_Sleep(10);
        // Process events
        while (const std::optional event = window.pollEvent()) {
            // Close window: exit
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                auto it = userThrottleMap.find(keyPressed->scancode);
                if (it != userThrottleMap.end()) {
                    // std::cout << "Accelerator at " << it->second << " \n";
                    gas = it->second;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
                    window.close();
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Up && !shiftLock) {
                    std::cout << "Upshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas(0);
                    upShiftFrame = frame + UPSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                    if (gasAvg.getAverage() > 100) {
                        engineCtx.fx.biquads[0].setPeakGain(0.0f);
                        engineCtx.fx.biquads[1].setPeakGain(0.0f);
                        engineCtx.fx.biquads[2].setPeakGain(0.0f);
                        engineCtx.fx.biquads[3].setPeakGain(9.0f + car.getRPM() / 500.0f);
                    }
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Down && !shiftLock) {
                    std::cout << "Downshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas((0.02 * car.getRPM()) + 30);
                    downShiftFrame = frame + DOWNSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::S) {
                    // Only allow starting if not already in the process of being started, otherwise we can attempt to start.
                    if (!isStarting) {
                        generalGen.startPlayback(1);
                        // Push to start.
                        std::cout << "Starting car...\n";
                        std::thread starterThread{carStarter, &car, &isStarting};
                        starterThread.detach();
                    }
                    isStarting = true;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::A) {
                    car.ignition = !car.ignition;
                    std::cout << "Set ignition to " << car.ignition << "\n";
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 5;
                }
            }
            if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                if (keyReleased->scancode == sf::Keyboard::Scancode::T) {
                    gas = 0;
                }
                if (keyReleased->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 0;
                }
                auto it = userThrottleMap.find(keyReleased->scancode);
                if (it != userThrottleMap.end()) {
                    // std::cout << "Accelerator at " << it->second << " \n";
                    gas = 0;
                }
            }
            if (const auto *joystickMove = event->getIf<sf::Event::JoystickMoved>()) {
                if (joystickMove->axis == sf::Joystick::Axis::Z) {
                    // std:: cout << "Moved: " << joystickMove->position << "\n";
                    gas = 0.75 * (100 - joystickMove->position);
                }
            }
            if (const auto *joystickButton = event->getIf<sf::Event::JoystickButtonPressed>()) {
                // Handle button presses
                if (joystickButton->button == 4 && !shiftLock) { // LB button
                    std::cout << "Upshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas(0);
                    upShiftFrame = frame + UPSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                    if (gasAvg.getAverage() > 100) {
                        engineCtx.fx.biquads[0].setPeakGain(0.0f);
                        engineCtx.fx.biquads[1].setPeakGain(0.0f);
                        engineCtx.fx.biquads[2].setPeakGain(0.0f);
                        engineCtx.fx.biquads[3].setPeakGain(car.getRPM() / 900.0f);
                    }
                } else if (joystickButton->button == 5 && !shiftLock) { // RB button
                    std::cout << "Downshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas((0.02 * car.getRPM()) + 30);
                    downShiftFrame = frame + DOWNSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                }
            }
        }
        if (upShiftFrame == 0 && downShiftFrame == 0 && !isStarting) {
            gasAvg.addValue(gas);
            car.setGas(gasAvg.getAverage());
        }

        if (frame == upShiftFrame) {
            car.setGear(std::clamp(lastGear + 1, 0, 7));
            upShiftFrame = 0;
            shiftLock = false;
            backfire.triggerPop();
            engineCtx.fx.biquads[0].setPeakGain(-5.0f);
            engineCtx.fx.biquads[1].setPeakGain(10.0f);
            engineCtx.fx.biquads[2].setPeakGain(3.0f);
            engineCtx.fx.biquads[3].setPeakGain(9.0f);
        }

        if (frame == downShiftFrame) {
            car.setGear(std::clamp(lastGear - 1, 0, 7));
            downShiftFrame = 0;
            shiftLock = false;
        }
        if (car.getGas() <= 10 && (lastLiftOff + 1000 / deltaTime >= frame)) {
            backfire.setIntensity(1.0f - ((frame - lastLiftOff) / (1000 / deltaTime)));
        }
        if (car.getRPM() <= 4000 || car.getGas() >= 25) {
            backfire.setIntensity(0);
        }
        if (car.getBoost() <= 15 && blowoff == false) {
            blowoff = true;
            turboGen.startPlayback(1);
            lastLiftOff = frame;
        }
        if (car.getBoost() >= 50) {
            blowoff = false;
        }

        // TODO: Seperation of concerns (Boost logic, shift styles etc.)
        engine.setRPM(car.getRPM());
        engineAlt.setRPM(car.getRPM());
        engineAltAlt.setRPM(car.getRPM());
        engine.setAmplitude(car.getTorque() / 100 + 0.2f);
        engineAlt.setAmplitude((car.getRPM() * car.getTorque()) / 5000000);
        engineAltAlt.setAmplitude(car.getRPM()/85000);
        whoosh.setIntensity(car.getBoost() / 150);
        whoosh.setAmplitude(car.getBoost() / 2500);
        turboShaft.setAmplitude(car.getBoost() / 750);
        turboShaft.setRPM(10000 + car.getBoost() * 100);
        supercharger.setRPM(car.getRPM()); // Supercharger example
        supercharger.setAmplitude(car.getRPM() / 100000 + 0.1f);
        // Gear whine example
        // supercharger.setNoteOffset(25+car.getGear());
        // supercharger.setRPM(car.getRPM()*(car.gearRatios[car.getGear()]/5)+1000);

        // Update tachometer needle rotation according to rpm.
        tach.setRotation(sf::degrees(car.getRPM() / 27.5 - 90));
        gaugeValue.setString("RPM: " + std::to_string((int)engine.getRPM()) + "\nBoost: " + std::to_string((int)car.getBoost()) + "\nGear: " + std::to_string((int)car.getGear()) + "\nSpeed: " + std::to_string((int)(car.getWheelSpeed() * 3.6)));
        //std::cout << "RPM: " << (int)engine.getRPM() << "  boost: " << car.getBoost() << " Gear: " << car.getGear() << " Speed: " << car.getWheelSpeed() * 3.6 << "\n";

        // ==== RENDERING
        window.clear();
        window.draw(spriteTach);
        window.draw(gaugeValue);
        window.draw(tach);
        window.draw(spriteMiddle);
        window.display();
    }

    // ==== EXIT ====
    carRunning = false;
    carThread.join();
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
