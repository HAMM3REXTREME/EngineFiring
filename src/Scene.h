#pragma once

#include "AudioContext.h"
#include "EngineSoundGenerator.h"
#include "Map2D.h"
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
    AudioSceneManager() { registerMethods(); }

    struct MethodKey {
        std::type_index type;
        std::string name;
        bool operator<(const MethodKey &other) const { return type == other.type ? name < other.name : type < other.type; }
    };
    struct BoundMethod {
        std::function<void(const std::vector<float> &)> call;
    };

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

    void callMethod(const std::string &command, const std::vector<float> &args) { selectCall(command)->call(args); }
    std::optional<BoundMethod> selectCall(const std::string &command) {
        size_t dot = command.find('.');
        if (dot == std::string::npos)
            return std::nullopt;

        std::string objName = command.substr(0, dot);
        std::string method = command.substr(dot + 1);

        // lookup object
        auto it = loadedSoundGenerators.find(objName);
        if (it == loadedSoundGenerators.end())
            return std::nullopt;

        SoundGenerator *obj = it->second.get();
        std::type_index type = typeid(*obj);

        // lookup method in registry
        MethodKey mk{type, method};
        auto mit = methodRegistry.find(mk);
        if (mit == methodRegistry.end()) {
            mk = {typeid(SoundGenerator), method};
            mit = methodRegistry.find(mk);
            if (mit == methodRegistry.end())
                return std::nullopt;
        }

        auto fn = mit->second; // function(SoundGenerator*, const vector<float>&)

        BoundMethod bound;
        // Pre-allocate vector outside lambda to avoid allocation each call
        std::vector<float> argsBuffer;

        bound.call = [obj, fn, argsBuffer = std::move(argsBuffer)](const std::vector<float> &values) mutable {
            argsBuffer = values; // copy values into pre-allocated buffer
            fn(obj, argsBuffer);
        };

        return bound;
    }

    std::unordered_map<std::string, float> vehicle_input;

    std::unordered_map<std::string, std::unique_ptr<Map2D>> loadedMap2Ds;

    void applyMap2D(const std::string &map2d_id) {
        std::string xKey = loadedMap2Ds[map2d_id]->xString;
        std::string yKey = loadedMap2Ds[map2d_id]->yString;
        std::string callCommand = loadedMap2Ds[map2d_id]->zString;
        std::vector<float> callArgs(1);
        std::cout << "ApplyMap: X: " << vehicle_input[xKey] << " Y: " << vehicle_input[yKey] << "\n";
        callArgs[0] = (loadedMap2Ds[map2d_id]->getValue(vehicle_input[xKey], vehicle_input[yKey]));
        std::cout << "Apply map: " << callArgs[0] << "\n";
        callMethod(callCommand, callArgs);
    }
    void applyAll2DMaps(){
        for (const auto &[key, value] : loadedMap2Ds){
            applyMap2D(key);
        }
    }

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

  private:
    // maps (type, methodName) → callable(SoundGenerator*, args)
    std::map<MethodKey, std::function<void(SoundGenerator *, const std::vector<float> &)>> methodRegistry;
    void registerMethods() {
        methodRegistry[{typeid(SoundGenerator), "setAmplitude"}] = [](SoundGenerator *obj, const std::vector<float> &args) {
            if (args.empty()) {
                return;
            }
            obj->setAmplitude(args[0]);
            std::cout << "Dynamically set amplitude to " << args[0] << "\n";
        };
        methodRegistry[{typeid(EngineSoundGenerator), "setRPM"}] = [](SoundGenerator *obj, const std::vector<float> &args) {
            auto *d = dynamic_cast<EngineSoundGenerator *>(obj);
            if (!d || args.empty()) {
                return;
            }
            d->setRPM(args[0]);
        };
    }
};