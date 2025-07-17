#pragma once
#include <iostream>

#include "AudioContext.h"
#include "AudioVector.h"
#include "BackfireSoundGenerator.h"
#include "Car.h"
#include "Damper.h"
#include "Engine.h"
#include "EngineSoundGenerator.h"
#include "SimpleSoundGenerator.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include "TurboWhooshGenerator.h"

#include "AudioContext.h"
#include "Car.h"
#include "Engine.h"
#include "SoundGenerator.h"
#include <memory>
#include <vector>

class AudioScene {
  public:
    std::vector<AudioContext> audioContexts;
    std::vector<SoundGenerator> soundGenerators;
    std::vector<Engine> engineDefs;
    std::vector<SoundBank> SoundBanks;

    AudioScene() = default;

    // Convenience functions to add new objects
    template <typename T, typename... Args> std::shared_ptr<T> addSoundGenerator(Args &&...args) {
        static_assert(std::is_base_of<SoundGenerator, T>::value, "Must be a SoundGenerator");
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        soundGenerators.push_back(ptr);
        return ptr;
    }

    AudioContext *findAudioContext(const std::string &id) {
        for (auto &context : audioContexts) {
            if (context.id == id) {
                return &context;
            }
        }
        return nullptr;
    }

    void addEngineDef(const Engine &engineDef) { engineDefs.push_back(engineDef); }

    void update() {}
};
