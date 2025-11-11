#pragma once

#include "AudioContext.h"
#include "EngineSoundGenerator.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include <filesystem>
#include <memory>
#include <unordered_map>
class Scene {
  public:
    float rpm = 0.0f;
    float load = 0.0f;

    std::unordered_map<std::string, std::unique_ptr<SoundBank>> loadedSoundBanks;
    std::unordered_map<std::string, std::unique_ptr<Engine>> engineDefinitions;
    std::unordered_map<std::string, std::unique_ptr<SoundGenerator>> loadedSoundGenerators;
    AudioContext mainCtx;
    void debugPrintGenerators() {
        for (const auto &[key, value] : loadedSoundGenerators) {
            std::cout << "SoundGenerators: '" << key << "' with info " << value->getInfo(0) << "\n";
        }
    }
    std::vector<std::string> loadRelativePaths(const std::filesystem::path &listFilePath) {
        std::vector<std::string> result;
        std::ifstream file(listFilePath);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + listFilePath.string());
        }

        std::filesystem::path baseDir = listFilePath.parent_path();
        std::string line;

        while (std::getline(file, line)) {
            // Skip empty lines or lines that start with '#'
            if (line.empty() || line[0] == '#')
                continue;

            // Trim potential trailing \r (Windows files)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            // Combine base directory + relative path
            std::filesystem::path fullPath = baseDir / line;
            result.push_back(fullPath.lexically_normal().string());
        }

        return result;
    }
    AudioContext &getMainCtx() { return mainCtx; }
    const AudioContext &getMainCtx() const { return mainCtx; }
    void loadSoundBank(const std::string &name, const std::string &importFilename) {
        try {
            auto paths = loadRelativePaths(importFilename); // "assets/audio/tick_library.sb"
            loadedSoundBanks.emplace(name, std::make_unique<SoundBank>(paths));
            std::cout << "Loaded soundbank from " << importFilename << "\n";
        } catch (const std::exception &e) {
            std::cerr << e.what() << "\n";
        }
    }
    void newEngineDef(const std::string &name, const std::string &firing_order, float firing_per_rev = 0.5) {
        engineDefinitions.emplace(name, std::make_unique<Engine>(name, Engine::getFiringOrderFromString(firing_order), firing_per_rev));
    }
    void newEngineDef(const std::string &name, const std::string &firing_order, const std::string &firing_interval, float firing_per_rev = 0.5) {
        engineDefinitions.emplace(name, std::make_unique<Engine>(name, Engine::getFiringOrderFromString(firing_order),
                                                                 Engine::getFiringIntervalFromString(firing_interval), firing_per_rev));
    }
    void newEngineSoundGenerator(const std::string &name, const std::string &soundbank_id, const std::string &engine_id) {
        loadedSoundGenerators.emplace(name, std::make_unique<EngineSoundGenerator>(*loadedSoundBanks[soundbank_id], *engineDefinitions[engine_id]));
    }
    void addToMainCtx(const std::string &generator_id) { mainCtx.addGenerator(loadedSoundGenerators[generator_id].get()); }
    void newAudioCtx(const std::string &name, float amplitude = 1.0f) { loadedSoundGenerators.emplace(name, std::make_unique<AudioContext>(amplitude)); }
    void addToAudioCtx(const std::string &ctx_id, const std::string &generator_id) {
        auto it = loadedSoundGenerators.find(ctx_id);
        if (it != loadedSoundGenerators.end()) {
            if (AudioContext *targetCtx = dynamic_cast<AudioContext *>(it->second.get())) {
                targetCtx->addGenerator(loadedSoundGenerators.at(generator_id).get());
            } else {
                std::cerr << "Warning: audio context not found.\n";
            }
        }
    }
};