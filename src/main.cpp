#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <portaudio.h>
#include <sndfile.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "AudioContext.h"
#include "AudioScene.h"
#include "BackfireSoundGenerator.h"
#include "Biquad.h"
#include "Car.h"
#include "Damper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SecondOrderFilter.h"
#include "SimpleSoundGenerator.h"
#include "SoundBank.h"
#include "TurboWhooshGenerator.h"

constexpr float SAMPLE_RATE = 48000;
constexpr int WINDOW_X = 1080;
constexpr int WINDOW_Y = 720;

constexpr int DOWNSHIFT_DELAY = 150;
constexpr int UPSHIFT_DELAY = 160;

constexpr float THROTTLE_BLIP_DOWN = 0.02f;

// Function for the PortAudio audio callback
int audio_callback(const void *, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *, PaStreamCallbackFlags, void *userData) {
    auto *audioContext = static_cast<AudioContext *>(userData);
    float *out = static_cast<float *>(outputBuffer);

    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        *out++ = std::sin((M_PI / 2.0f) * audioContext->getAllSamples());
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
    bool showDebug = false;

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

    // Engine engineDef("Revuelto V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 7);
    // Engine engineDef("Diablo/Murci V12", Engine::getFiringOrderFromString("1-7-4-10-2-8-6-12-3-9-5-11"), 6);
    // Engine engineDef("Countach V12", Engine::getFiringOrderFromString("1 7 5 11 3 9 6 12 2 8 4 10"), 5);
    // Engine engineDef("BMW S70/2 V12", Engine::getFiringOrderFromString("1 7 5 11 3 9 6 12 2 8 4 10"), 6.2);
    // Engine engineDef("F1 V12", {0, 11, 3, 8, 1, 10, 5, 6, 2, 9, 4, 7}, 16);
    // Engine engineDef("Audi V10 FSI", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {90, 54, 90, 54, 90, 54, 90, 54, 90, 54}, 5);
    Engine engineDef("1LR-GUE V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 5);
    // Engine engineDef("M80 V10",{0, 5, 4, 9, 1, 6, 2, 7, 3, 8},{70,74,70,74,70,74,70,74,70,74,70} , 5);
    // Engine engineDef("Random V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, {72,73,74,75,76,77,78,79,80,81}, 5);
    // Engine engineDef("Mercedes AMG M156", Engine::getFiringOrderFromString("1-5-4-2-6-3-7-8"),4);
    // Engine engineDef("F1 V10", {0, 5, 4, 9, 1, 6, 2, 7, 3, 8}, 12.5);
    // Engine engineDef("Bugatti W16", Engine::getFiringOrderFromString("1 14 9 4 7 12 15 6 13 8 3 16 11 2 5 10"), 8.2);
    // Engine engineDef("Inline 9 - Experimental", Engine::getFiringOrderFromString("1 2 4 6 8 9 7 5 3"), 5);
    // Engine engineDef("Flat plane V8", Engine::getFiringOrderFromString("1 5 3 7 4 8 2 6"), 4);
    // Engine engineDef("Mercedes M120 V12", Engine::getFiringOrderFromString("1-12-5-8-3-10-6-7-2-11-4-9"),7.6);
    // Engine engineDef("Murican V8 +", Engine::getFiringOrderFromString("1-8-7-2-6-5-4-3"),3);
    // Engine engineDef("2UR-GSE V8", Engine::getFiringOrderFromString("1-8-7-3-6-5-4-2"),4);
    // Engine engineDef("BMW N54", Engine::getFiringOrderFromString("1-5-3-6-2-4"), 3);
    // Engine engineDef("Diesel inline 6", Engine::getFiringOrderFromString("1-5-3-6-2-4"), 1);
    // Engine engineDef("V Twin", Engine::getFiringOrderFromString("1-2"), {315,405},0.8);
    // Engine engineDef("1 Cylinder", {0}, 0.5);
    // Engine engineDef("3 Cylinder Sport", Engine::getFiringOrderFromString("1-2-3"),2);
    // Engine engineDef("VR6", Engine::getFiringOrderFromString("1-5-3-6-2-4"), {120, 130, 110, 125, 115, 120}, 3);
    // Engine engineDef("Audi i5", Engine::getFiringOrderFromString("1-2-4-5-3"),2);
    // Engine engineDef("Perfect Fifth i5", Engine::getFiringOrderFromString("1 3 5 2 4"), {120, 180, 120, 180, 120},3);
    // Engine engineDef("4 Banger", Engine::getFiringOrderFromString("1-3-4-2"),2);
    // Engine engineDef("Super Sport", Engine::getFiringOrderFromString("1-3-4-2"),4);
    // Engine engineDef("Cross plane i4 moto", Engine::getFiringOrderFromString("1-3-2-4"), {180,90,180,270},4);
    // Engine engineDef("Ducati V4", Engine::getFiringOrderFromString("1-2-4-3"),{90,200,90,340}, 4);
    // Engine engineDef("Nissan VQ", Engine::getFiringOrderFromString("1-2-3-4-5-6"),3);
    // Engine engineDef("Nissan VQ - Unequal headers", Engine::getFiringOrderFromString("1-2-3-4-5-6"), {177,183,177,183,177,183}, 2.8);
    // Engine engineDef("Ford 4.0L V6 / Honda C-series 90", Engine::getFiringOrderFromString("1-4-2-5-3-6"),3);
    // Engine engineDef("Ferrari V6", Engine::getFiringOrderFromString("1-6-3-4-2-5"),3.5);
    // Engine engineDef("F1 V6", Engine::getFiringOrderFromString("1-4-2-5-3-6"),6);
    // Engine engineDef("Buick even firing V6", Engine::getFiringOrderFromString("1-6-5-4-3-2"),3);
    // Engine engineDef("Buick odd firing V6", Engine::getFiringOrderFromString("1-6-5-4-3-2"), {90,150,90,150,90,150},3);
    // Engine engineDef("Porsche Flat 6", Engine::getFiringOrderFromString("1-6-2-4-3-5"), 3.6);
    EngineSoundGenerator engineLowNote(mainSamples, engineDef, 1000.0f, 0.5f);
    EngineSoundGenerator engineHighNote(mainSamples, engineDef, 1000.0f, 0.5f);
    EngineSoundGenerator engineMechanicals(mainSamples, engineDef, 1000.0f, 0.5f);
    engineLowNote.setNoteOffset(0); // 0
    engineHighNote.setNoteOffset(9); // 5 , 9, 19
    engineMechanicals.setNoteOffset(11); // 8, 11, 16

    // EQ Tips:
    // 1. Filter out any harsh harmonics (extremes of hearing range)
    // 2. Play around until it sounds right at lower RPMs
    // 3. Adjust high-pitched engine sound component to be at a screaming note offset (to get that screaming note over the base)

    // ==== SUPERCHARGER (Just a high revving 1 cylinder)
    Engine superchargerDef("Supercharger", {0}, 8.0f);
    EngineSoundGenerator supercharger(mainSamples, superchargerDef, 1000.0f, 0.7f);
    supercharger.setAmplitude(0.5f);
    supercharger.setNoteOffset(20);

    // ==== GENERAL SOUND SAMPLES
    SoundBank generalSamples;
    generalSamples.addFromWavs({"assets/audio/extra/boom.wav", "assets/audio/extra/starter.wav"});
    SimpleSoundGenerator generalGen(generalSamples);
    generalGen.setAmplitude(0.3f);

    // ==== TURBOCHARGER SHAFT (Sounds like a faint supercharger)
    Engine turboshaftDef("BorgWarner K04 - Shaft", {0}, 8.0f);
    EngineSoundGenerator turboShaft(mainSamples, turboshaftDef, 1000.0f, 0.05f);

    // ==== TURBO WHOOSH NOISE GENERATOR
    TurboWhooshGenerator whoosh(SAMPLE_RATE);

    // ==== TURBO FLUTTER AND BOV SOUNDS ETC.
    SoundBank turboSamples;
    turboSamples.addFromWavs({"assets/audio/extra/thump.wav", "assets/audio/extra/bov.wav"});
    SimpleSoundGenerator turboGen(turboSamples);
    turboGen.setAmplitude(0.25f);

    // ==== BACKFIRE NOISE GENERATOR
    BackfireSoundGenerator backfire(SAMPLE_RATE);
    backfire.setAmplitude(0.4f);

    // Audio sample generators that get summed up and played together
    AudioContext engineCtx("engines", {&engineLowNote, &engineHighNote, &engineMechanicals});
    AudioContext backfireCtx("backfire", {&backfire});
    AudioContext superchargerCtx("supercharger", {&supercharger});
    AudioContext context("root", {&engineCtx, &generalGen, &backfireCtx});

    // Car simulator stuff
    Car car;
    std::atomic<bool> carRunning = true;
    std::thread carThread{manageCar, &car, &carRunning};
    car.ignition = false;
    car.revLimiterCutTicks = 2;

    // TODO: Shifting + Post process wrapper class
    float carRpm = 0;    // Processed RPM
    float carTorque = 0; // Processed Torque
    Biquad torqueFilter(bq_type_lowpass, 25.0f / 60.0f, 0.707f, 0.0f);
    SecondOrderFilter rpmFilter(5.0f, 0.3f, 0.01);
    bool isStarting = false;
    bool blowoff = false;
    bool lifted = false;
    float gas = 0;
    int frame = 0;    // Frame counter since program started
    int lastGear = 0; // Last (non-fake-clutch) gear
    int upShiftFrame = 0;
    int downShiftFrame = 0;
    int lastLiftOff = 0;
    bool shiftLock = false; // Only allow one type of shift at a time (upshift/downshift)
    Damper gasAvg(5);       // Smooth out gas inputs

    // Biquad filters
    Biquad lowShelfFilter(bq_type_lowshelf, 150.0f / SAMPLE_RATE, 0.707f, 0.0f);  // cut lows
    Biquad midBoostFilter(bq_type_peak, 1500.0f / SAMPLE_RATE, 1.0f, 3.0f);       // boost mids
    Biquad highShelfFilter(bq_type_highshelf, 2500.0f / SAMPLE_RATE, 1.0f, 8.0f); // boost highs
    Biquad rumbleBoostFilter(bq_type_peak, 80.0f / SAMPLE_RATE, 0.5f, 10.1f);     // deep bass boost
    // Cuts
    Biquad cut22Hz(bq_type_peak, 22.0f / SAMPLE_RATE, 4.36f, -36.0f);
    Biquad cut28Hz(bq_type_peak, 28.0f / SAMPLE_RATE, 4.36f, -16.0f);
    Biquad cut18100Hz(bq_type_peak, 18100.0f / SAMPLE_RATE, 4.36f, -26.0f);
    Biquad cut14600Hz(bq_type_peak, 14600.0f / SAMPLE_RATE, 4.36f, -14.0f);
    // Boosts
    Biquad boost2100Hz(bq_type_peak, 2100.0f / SAMPLE_RATE, 4.36f, +2.0f);
    Biquad boost2600Hz(bq_type_peak, 2600.0f / SAMPLE_RATE, 4.36f, +2.0f);
    Biquad boost3600Hz(bq_type_peak, 3600.0f / SAMPLE_RATE, 4.36f, +2.0f);
    Biquad boost4000Hz(bq_type_peak, 4000.0f / SAMPLE_RATE, 4.36f, +2.0f);
    // Adding some filters
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 1600.0f / SAMPLE_RATE, 0.707f, 0.0f)); // Example active filter (dB modulated later)
    engineCtx.fx.addFilter(midBoostFilter);
    superchargerCtx.fx.addFilter(rumbleBoostFilter);
    // Filter out harsh sounds
    engineCtx.fx.addFilter(cut22Hz);
    engineCtx.fx.addFilter(cut28Hz);
    engineCtx.fx.addFilter(cut18100Hz);
    engineCtx.fx.addFilter(cut14600Hz);
    superchargerCtx.fx.addFilter(cut22Hz);
    superchargerCtx.fx.addFilter(cut28Hz);
    superchargerCtx.fx.addFilter(cut18100Hz);
    // Low ends
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 120.0f / SAMPLE_RATE, 0.707f, 4.0f));
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 300 / SAMPLE_RATE, 1.0f, 3.0f));
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 500.0f / SAMPLE_RATE, 0.707f, 3.0f));
    // Mid range
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 1700.0f / SAMPLE_RATE, 0.707f, 4.0f));
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 3000.0f / SAMPLE_RATE, 0.707f, 4.0f));
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 8000.0f / SAMPLE_RATE, 0.707f, -2.0f)); // High note - crispyness
    engineCtx.fx.addFilter(Biquad(bq_type_peak, 9000.0f / SAMPLE_RATE, 0.707f, -3.0f));
    // Backfire context
    Biquad backfireFilter(bq_type_lowshelf, 150.0f / SAMPLE_RATE, 0.707f, 12.0f);
    Biquad backfireHighFilter(bq_type_highshelf, 3000.0f / SAMPLE_RATE, 0.707f, -12.0f);
    Biquad backfireHighFilter2(bq_type_peak, 4500.0f / SAMPLE_RATE, 0.3f, -12.0f);
    backfireCtx.fx.addFilter(backfireFilter);
    backfireCtx.fx.addFilter(rumbleBoostFilter);
    backfireCtx.fx.addFilter(midBoostFilter);
    backfireCtx.fx.addFilter(cut14600Hz);
    // backfireCtx.fx.addFilter(cut18100Hz);
    backfireCtx.fx.addFilter(boost2100Hz);
    backfireCtx.fx.addFilter(backfireHighFilter);
    // backfireCtx.fx.addFilter(backfireHighFilter2);

    // SFML stuff for UI + input
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
    sf::Text debugText(font, "Debug text", 16);
    debugText.setFillColor(sf::Color::Green);
    debugText.setPosition({25.f, 25.f});

    // PortAudio for live audio playback
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256, audio_callback, &context);
    Pa_StartStream(stream);

    //  ==== APPLICATION MAIN LOOP ====
    while (window.isOpen()) {
        frame++;
        // Time since last frame
        float deltaTime = deltaClock.restart().asMilliseconds();
        Pa_Sleep(10);
        // Process events
        while (const std::optional event = window.pollEvent()) {
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
                if (keyPressed->scancode == sf::Keyboard::Scancode::D) {
                    showDebug = !showDebug;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Up && !shiftLock) {
                    std::cout << "Upshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas(0);
                    upShiftFrame = frame + UPSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                    if (gasAvg.getAverage() > 100) {
                        // engineCtx.fx.biquads[3].setPeakGain(10.1f + car.getRPM() / 500.0f);
                    }
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Down && !shiftLock) {
                    std::cout << "Downshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas((THROTTLE_BLIP_DOWN * car.getRPM()) + 30);
                    downShiftFrame = frame + DOWNSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::S && !isStarting) {
                    // If we've not already started the startup procedure, do the startup procedure
                    generalGen.startPlayback(1);
                    // Push to start.
                    std::cout << "Starting car...\n";
                    std::thread starterThread{carStarter, &car, &isStarting};
                    starterThread.detach();
                    isStarting = true;
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::A) {
                    car.ignition = !car.ignition;
                    std::cout << "Ignition: " << car.ignition << "\n";
                }
                if (keyPressed->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 5; // Brakes
                }
            }
            if (const auto *keyReleased = event->getIf<sf::Event::KeyReleased>()) {
                if (keyReleased->scancode == sf::Keyboard::Scancode::Period) {
                    car.linearWheelDrag = 0;
                }
                auto it = userThrottleMap.find(keyReleased->scancode);
                if (it != userThrottleMap.end()) {
                    gas = 0;
                }
            }
            if (const auto *joystickMove = event->getIf<sf::Event::JoystickMoved>()) {
                if (joystickMove->axis == sf::Joystick::Axis::Z) {
                    // std:: cout << "Axis::Z " << joystickMove->position << "\n";
                    gas = 0.75 * (100 - joystickMove->position);
                }
                if (joystickMove->axis == sf::Joystick::Axis::R) {
                    // std:: cout << "Axis::R " << joystickMove->position << "\n";
                    car.linearWheelDrag = 0.03 * (100 - joystickMove->position);
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
                } else if (joystickButton->button == 5 && !shiftLock) { // RB button
                    std::cout << "Downshift\n";
                    lastGear = car.getGear();
                    car.setGear(0);
                    car.setGas((THROTTLE_BLIP_DOWN * car.getRPM()) + 30);
                    downShiftFrame = frame + DOWNSHIFT_DELAY / deltaTime;
                    shiftLock = true;
                }
            }
        }
        // TODO: Wrap in post processing + shifting etc. class
        if (upShiftFrame == 0 && downShiftFrame == 0 && !isStarting) {
            gasAvg.addValue(gas);
            car.setGas(gasAvg.getAverage());
        }
        if (frame == upShiftFrame) {
            car.setGear(std::clamp(lastGear + 1, 0, 7));
            upShiftFrame = 0;
            shiftLock = false;
            backfire.triggerPop();
        }
        if (frame == downShiftFrame) {
            car.setGear(std::clamp(lastGear - 1, 0, 7));
            downShiftFrame = 0;
            shiftLock = false;
            backfire.triggerPop();
        }

        // Simple turbocharger state logic
        if (car.getGas() <= 10 && !blowoff) {
            blowoff = true;
            turboGen.startPlayback(1);
        }
        if (car.getBoost() >= 100) {
            blowoff = false;
        }

        // Simple supercharger state logic
        if (car.getGas() <= 10 && !lifted) {
            lastLiftOff = frame;
            lifted = true;
            supercharger.setNoteOffset(10);
        }
        if (car.getGas() >= 15) {
            lifted = false;
            supercharger.setNoteOffset(20);
        }

        // Simple backfire state logic
        if (car.getGas() <= 10 && (lastLiftOff + 600 / deltaTime >= frame)) {
            backfire.setIntensity(1.0f - ((frame - lastLiftOff) / (600 / deltaTime)));
        }
        if (car.getRPM() <= 4000 || car.getGas() >= 25) {
            backfire.setIntensity(0);
        }
        // TODO: Seperation of concerns (Boost logic, shift styles etc.)
        carRpm = rpmFilter.update(car.getRPM());
        carTorque = torqueFilter.process(car.getTorque());
        engineCtx.fx.biquads[0].setPeakGain(8.0f - (carTorque / 20.0f));
        engineCtx.setAmplitude(0.5f + carRpm / 20000.0f);
        engineLowNote.setRPM(carRpm);
        engineHighNote.setRPM(carRpm);
        engineMechanicals.setRPM(carRpm);
        engineLowNote.setAmplitude(carTorque / 175 + 0.1f);
        engineHighNote.setAmplitude((carRpm * carTorque) / 1000000);
        engineMechanicals.setAmplitude(carRpm / 80000);
        whoosh.setIntensity(car.getBoost() / 100);
        whoosh.setAmplitude(car.getBoost() / 2000);
        turboShaft.setAmplitude(car.getBoost() / 200);
        turboShaft.setRPM(10000 + car.getBoost() * 200);
        // == Supercharger example:
        // supercharger.setRPM(car.getRPM());
        // supercharger.setAmplitude(carTorque / 300);
        // == Gear whine example:
        // supercharger.setAmplitude(car.getGear() > 0 ? car.getWheelSpeed() / 500 : 0.0f);
        // supercharger.setRPM(carRpm * (car.gearRatios[car.getGear()] * 2) + 1000);

        // Update tachometer needle rotation according to rpm.
        tach.setRotation(sf::degrees(carRpm / 27.5 - 90));
        debugText.setString(context.getInfo(0));
        gaugeValue.setString("RPM: " + std::to_string((int)engineLowNote.getRPM()) + "\nBoost: " + std::to_string((int)car.getBoost()) +
                             "\nGear: " + std::to_string((int)car.getGear()) + "\nSpeed: " + std::to_string((int)(car.getWheelSpeed() * 3.6)));

        // ==== RENDERING
        window.clear();
        window.draw(spriteTach);
        window.draw(gaugeValue);
        window.draw(tach);
        window.draw(spriteMiddle);
        if (showDebug) {
            window.draw(debugText);
        }
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
