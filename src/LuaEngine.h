#pragma once
#include "sol/sol.hpp"

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

// Messy glue class to allow interacting with C++ stuff via lua

class LuaEngine {
  public:
    sol::state lua;
    sol::function initFunction;
    sol::function tickFunction;
    int runLuaScript(const std::string &filename) {
        try {
            lua.script_file(filename);
        } catch (const sol::error &e) {
            std::cerr << "Lua: Script error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }
    // Call this after your C++ setup is done
    void init() {
        initFunction = lua["init"];
        sol::protected_function_result pfr = initFunction();
        if (!pfr.valid()) {
            sol::error err = pfr;
            std::cerr << "Lua: Init function error: " << err.what() << std::endl;
        }
    }
    void tick() {
        tickFunction = lua["tick"];
        sol::protected_function_result pfr = tickFunction();
        if (!pfr.valid()) {
            sol::error err = pfr;
            std::cerr << "Lua: Tick function error: " << err.what() << std::endl;
        }
    }
    LuaEngine() {
        lua.open_libraries(sol::lib::base);
        lua["vecFloat"] = &tableToVectorFloat;
        lua["vecInt"] = &tableToVectorInt;
        lua["vecString"] = &tableToVectorString;
        tickFunction = lua["tick"];

        lua["bq_type_lowpass"] = 0;
        lua["bq_type_highpass"] = 1;
        lua["bq_type_bandpass"] = 2;
        lua["bq_type_notch"] = 3;
        lua["bq_type_peak"] = 4;
        lua["bq_type_lowshelf"] = 5;
        lua["bq_type_highshelf"] = 6;

        lua.new_usertype<Engine>("Engine",
                                 sol::constructors<Engine(std::string, const std::vector<int> &, const std::vector<float> &, float),
                                                   Engine(std::string, const std::vector<int> &, float)>(),
                                 "name", &Engine::name, "firingOrder", &Engine::firingOrder, "firingIntervalFactors", &Engine::firingIntervalFactors,
                                 "audioRpmFactor", &Engine::audioRpmFactor, "setIntervalsFromDegrees", &Engine::setIntervalsFromDegrees,
                                 "getFiringOrderFromString", sol::var(&Engine::getFiringOrderFromString), "getCylinderCount", &Engine::getCylinderCount);
        lua.new_usertype<AudioVector>("AudioVector",
                                      sol::constructors<AudioVector(), AudioVector(const std::string &), AudioVector(const std::vector<float> &)>(),
                                      "loadfromWav", &AudioVector::loadfromWav);
        lua.new_usertype<SoundBank>("SoundBank", sol::constructors<SoundBank()>(), "addFromWavs", &SoundBank::addFromWavs, "clearAll", &SoundBank::clearAll,
                                    "samples", &SoundBank::samples);
        lua.new_usertype<Biquad>("Biquad", sol::constructors<Biquad(), Biquad(int, double, double, double)>(), "setType", &Biquad::setType, "setQ",
                                 &Biquad::setQ, "setFc", &Biquad::setFc, "setPeakGain", &Biquad::setPeakGain, "setBiquad", &Biquad::setBiquad, "process",
                                 &Biquad::process);
        lua.new_usertype<EffectChain>("EffectChain", sol::constructors<EffectChain()>(), "addFilter", &EffectChain::addFilter, "clear", &EffectChain::clear,
                                      "process", &EffectChain::process,
                                      // Expose the vector of Biquads (optional, for inspection)
                                      "biquads", &EffectChain::biquads);
        lua.new_usertype<SoundGenerator>("SoundGenerator",
                                         // You can expose virtual functions if you want (pure virtual or not)
                                         "getSample", &SoundGenerator::getSample, "setAmplitude", &SoundGenerator::setAmplitude, "getAmplitude",
                                         &SoundGenerator::getAmplitude);
        lua.new_usertype<AudioContext>("AudioContext", sol::constructors<AudioContext(), AudioContext(std::vector<SoundGenerator *>)>(), "addGenerator",
                                       &AudioContext::addGenerator, "setAmplitude", &AudioContext::setAmplitude, "getAmplitude", &AudioContext::getAmplitude,
                                       "fx", &AudioContext::fx, "getSample", &AudioContext::getSample);
        lua.new_usertype<EngineSoundGenerator>(
            "EngineSoundGenerator",
            // Constructor: (const SoundBank&, const Engine&, float rpm, float max_amplitude = 0.5f, int sample_rate = 48000, int channels = 1)
            sol::constructors<EngineSoundGenerator(const SoundBank &, const Engine &, float),
                              EngineSoundGenerator(const SoundBank &, const Engine &, float, float),
                              EngineSoundGenerator(const SoundBank &, const Engine &, float, float, int),
                              EngineSoundGenerator(const SoundBank &, const Engine &, float, float, int, int)>(),
            sol::base_classes, sol::bases<SoundGenerator>(),
            // Inherited SoundGenerator methods
            "getSample", &EngineSoundGenerator::getSample, "setAmplitude", &EngineSoundGenerator::setAmplitude, "getAmplitude",
            &EngineSoundGenerator::getAmplitude,
            // EngineSoundGenerator methods
            "setRPM", &EngineSoundGenerator::setRPM, "getRPM", &EngineSoundGenerator::getRPM, "setNoteOffset", &EngineSoundGenerator::setNoteOffset,
            "getNoteOffset", &EngineSoundGenerator::getNoteOffset, "update", &EngineSoundGenerator::update);
    }
    static std::vector<float> tableToVectorFloat(sol::table t) {
        std::vector<float> result;

        t.for_each([&](sol::object key, sol::object value) {
            // Only process integer keys (array portion)
            if (key.is<int>()) {
                if (value.is<float>()) {
                    result.push_back(value.as<float>());
                } else {
                    std::cerr << "Warning: Non-float value at index " << key.as<int>() << " ignored\n";
                }
            }
        });
        return result;
    }
    static std::vector<int> tableToVectorInt(sol::table t) {
        std::vector<int> result;

        t.for_each([&](sol::object key, sol::object value) {
            if (key.is<int>()) {
                if (value.is<int>()) {
                    result.push_back(value.as<int>());
                } else if (value.is<float>()) {
                    // Convert float to int (truncates decimal part)
                    result.push_back(static_cast<int>(value.as<float>()));
                    std::cerr << "Notice: Converted float to int at index " << key.as<int>() << "\n";
                } else {
                    std::cerr << "Warning: Non-numeric value at index " << key.as<int>() << " ignored\n";
                }
            }
        });
        return result;
    }
    static std::vector<std::string> tableToVectorString(sol::table t) {
        std::vector<std::string> result;

        t.for_each([&](sol::object key, sol::object value) {
            // Only process integer keys (array portion)
            if (key.is<int>()) {
                if (value.is<std::string>()) {
                    result.push_back(value.as<std::string>());
                } else if (value.is<int>() || value.is<float>()) {
                    // Convert numbers to strings
                    std::ostringstream oss;
                    if (value.is<int>()) {
                        oss << value.as<int>();
                    } else {
                        oss << value.as<float>();
                    }
                    result.push_back(oss.str());
                    std::cerr << "Notice: Converted numeric value to string at index " << key.as<int>() << "\n";
                } else {
                    std::cerr << "Warning: Non-string value at index " << key.as<int>() << " ignored\n";
                }
            }
        });

        return result;
    }
};