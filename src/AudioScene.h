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
    // Use shared_ptr or unique_ptr to manage polymorphic SoundGenerator objects safely
    std::vector<std::shared_ptr<AudioContext>> audioContexts;

    // Store polymorphic sound generators as pointers for polymorphism
    std::vector<std::shared_ptr<SoundGenerator>> soundGenerators;

    // Store engine definitions
    std::vector<Engine> engineDefs;

    AudioScene() = default;

    // Convenience functions to add new objects
    template <typename T, typename... Args> std::shared_ptr<T> addSoundGenerator(Args &&...args) {
        static_assert(std::is_base_of<SoundGenerator, T>::value, "Must be a SoundGenerator");
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        soundGenerators.push_back(ptr);
        return ptr;
    }

    std::shared_ptr<AudioContext> addAudioContext(const std::vector<SoundGenerator *> &generators) {
        auto ctx = std::make_shared<AudioContext>(generators);
        audioContexts.push_back(ctx);
        return ctx;
    }

    void addEngineDef(const Engine &engineDef) { engineDefs.push_back(engineDef); }

    void update() {}
};
