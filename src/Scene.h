#pragma once

#include "AudioContext.h"
#include "EngineSoundGenerator.h"
#include "Map2D.h"
#include "SineClipper.h"
#include "SoundBank.h"
#include "SoundGenerator.h"
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <typeindex>
#include <unordered_map>
class AudioSceneManager {
  public:
    struct FileEntry {
        std::string path;
        std::string name; // optional
    };

    static std::vector<FileEntry> loadRelativePathsWithNames(const std::filesystem::path &listFilePath) {
        std::vector<FileEntry> result;
        std::ifstream file(listFilePath);

        if (!file.is_open()) {
            throw std::runtime_error("Failed to open " + listFilePath.string());
        }

        std::filesystem::path importFileDir = listFilePath.parent_path();
        std::string line;

        while (std::getline(file, line)) {
            // Skip empty lines or lines that start with '#'
            if (line.empty() || line[0] == '#')
                continue;

            // Trim potential trailing \r (Windows files)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            // Trim leading/trailing whitespace
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);

            if (line.empty())
                continue;

            // Split line into path + optional name
            std::istringstream iss(line);
            std::string relPath;
            iss >> relPath;

            std::string optionalName;
            std::getline(iss, optionalName);
            optionalName.erase(0, optionalName.find_first_not_of(" \t"));

            FileEntry entry;
            entry.path = (importFileDir / relPath).lexically_normal().string();
            entry.name = optionalName;

            result.push_back(entry);
        }

        return result;
    }


    std::unordered_map<std::string, float> vehicleInput;

    std::unordered_map<std::string, std::unique_ptr<Map2D>> loadedMap2Ds;
    std::unordered_map<std::string, std::unique_ptr<SoundBank>> loadedSoundBanks;
    std::unordered_map<std::string, std::unique_ptr<Engine>> engineDefinitions;
    std::unordered_map<std::string, std::unique_ptr<SoundGenerator>> loadedSoundGenerators;
    AudioContext mainCtx;
    void debugPrintGenerators() {
        for (const auto &[key, value] : loadedSoundGenerators) {
            std::cout << "SoundGenerators: '" << key << "' with info " << value->getInfo(0) << "\n";
        }
    }
    AudioContext &getMainCtx() { return mainCtx; }
    const AudioContext &getMainCtx() const { return mainCtx; }
    void loadSoundBank(const std::string &name, const std::string &sbFilename) {
        try {
            loadedSoundBanks.emplace(name, std::make_unique<SoundBank>(sbFilename));
            std::cout << "Loaded soundbank from " << sbFilename << "\n";
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
    void importMap2D(const std::string &name, const std::string &filename) { loadedMap2Ds.emplace(name, std::make_unique<Map2D>(filename)); }
    void importMapCollection(const std::string &mclFilename) {
        auto files = loadRelativePathsWithNames(mclFilename);
        for (const auto &f : files) {
            importMap2D(f.name, f.path);
            std::cout << f.path << " | " << f.name << "\n";
        }
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