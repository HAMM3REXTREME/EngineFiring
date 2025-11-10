#pragma once

#include "AudioContext.h"
#include "EngineSoundGenerator.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include <filesystem>
#include <memory>
class Scene {
  public:
  
    std::vector<std::unique_ptr<SoundBank>> loadedSoundBanks;
    std::vector<std::unique_ptr<Engine>> engineDefinitions;
    std::vector<std::unique_ptr<SoundGenerator>> loadedSoundGenerators;
    AudioContext mainCtx;
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
    void loadSoundBank(const std::string &importFilename) {
        try {
            auto paths = loadRelativePaths(importFilename); // "assets/audio/tick_library.sb"
            loadedSoundBanks.push_back(std::make_unique<SoundBank>(paths));
            std::cout << "Loaded soundbank from " << importFilename << "\n";
        } catch (const std::exception &e) {
            std::cerr << e.what() << "\n";
        }
    }
    void newEngineDef(const std::string& name, const std::string firing_order, float firing_per_rev=0.5){
        engineDefinitions.push_back(std::make_unique<Engine>(name,Engine::getFiringOrderFromString(firing_order),firing_per_rev));
    }
    void newEngineDef(const std::string& name, const std::string firing_order, const std::string firing_interval, float firing_per_rev=0.5){
        engineDefinitions.push_back(std::make_unique<Engine>(name,Engine::getFiringOrderFromString(firing_order), Engine::getFiringIntervalFromString(firing_interval),firing_per_rev));
    }
    void newEngineSoundGenerator(size_t soundbank_id, size_t engine_id){
        loadedSoundGenerators.push_back(std::make_unique<EngineSoundGenerator>(*loadedSoundBanks[soundbank_id], *engineDefinitions[engine_id]));
    }
    void addToMainCtx(size_t generator_id){
        mainCtx.addGenerator(loadedSoundGenerators[generator_id].get());
    }



};