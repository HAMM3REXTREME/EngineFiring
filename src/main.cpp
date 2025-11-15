#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Mouse.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <portaudio.h>
#include <sndfile.h>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "AudioContext.h"
#include "BackfireSoundGenerator.h"
#include "Biquad.h"
#include "Car.h"
#include "CarTriBlendScene.h"
#include "CubicClipper.h"
#include "Damper.h"
#include "DerivativeFilter.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "ForceSilenceFilter.h"
#include "Graph.h"
#include "HardClamp.h"
#include "Map2D.h"
#include "SaturatorFilter.h"
#include "Scene.h"
#include "SecondOrderFilter.h"
#include "SimpleSoundGenerator.h"
#include "SineClipper.h"
#include "SoundBank.h"
#include "TurboWhooshGenerator.h"

constexpr float SAMPLE_RATE = 48000;
constexpr int WINDOW_X = 1080;
constexpr int WINDOW_Y = 720;

constexpr int DOWNSHIFT_DELAY = 150;
constexpr int UPSHIFT_DELAY = 150;

constexpr float THROTTLE_BLIP_DOWN = 0.021f;

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
    car->setGas(70);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    car->setGas(0);
    *m_isStarting = false;
}

int main() {

    sf::RenderWindow window(sf::VideoMode({WINDOW_X, WINDOW_Y}), "Engine Firing Simulator");
    sf::Clock deltaClock;
    bool showDebug = false;

    CarTriBlendScene example;
    example.buildFromCfg("assets/cars/lfa/vehicle.cfg");

    // Car simulator stuff
    Car car;
    std::atomic<bool> carRunning = true;
    std::thread carThread{manageCar, &car, &carRunning};
    car.ignition = false;
    car.revLimiterCutTicks = 2;

    // TODO: Shifting + Post process wrapper class
    float carRpm = 0;    // Processed RPM
    float carTorque = 0; // Processed Torque
    Biquad torqueFilter(bq_type_lowpass, 25.0f / 100.0f, 0.707f, 0.0f);
    SecondOrderFilter rpmFilter(5.0f, 0.25f, 0.01);
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
    Damper gasAvg(0.333);   // Smooth out gas inputs
    DerivativeFilter turboDrop(100);

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

    // PortAudio for live audio playback
    Pa_Initialize();
    PaStream *stream;
    Pa_OpenDefaultStream(&stream, 0, 1, paFloat32, SAMPLE_RATE, 256, audio_callback, example.main_ctx.get());
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
                    // DSG Sound
                    // generalGen.setAmplitude(gas/300.0f + 0.1f);
                    // generalGen.startPlayback(2);
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
        }
        if (frame == downShiftFrame) {
            car.setGear(std::clamp(lastGear - 1, 0, 7));
            downShiftFrame = 0;
            shiftLock = false;
        }

        // TODO: Seperation of concerns (Boost logic, shift styles etc.)
        carRpm = rpmFilter.update(car.getRPM());
        carTorque = torqueFilter.process(car.getTorque());

        example.input_values["rpm"] = carRpm;
        example.input_values["load"] = carTorque;
        example.input_values["boost"] = car.getBoost();
        example.tick();
        // Update tachometer needle rotation according to rpm.
        tach.setRotation(sf::degrees(carRpm / 27.5 - 90));
        gaugeValue.setString("RPM: " + std::to_string((int)car.getRPM()) + "\nBoost: " + std::to_string((int)car.getBoost()) +
                             "\nGear: " + std::to_string((int)car.getGear()) + "\nSpeed: " + std::to_string((int)(car.getWheelSpeed() * 3.6)));

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
    std::cout << "Stopping PortAudio...";
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();
    std::cout << "Bye" << std::endl;

    return 0;
}
