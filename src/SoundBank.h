
#pragma once

#include "AudioVector.h"
#include <filesystem>
#include <iostream>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <typeindex>
#include <fstream>
#include <sstream>
#include <unordered_map>
class SoundBank {
  public:
    SoundBank();
    SoundBank(const std::vector<std::string> &filenames);
    SoundBank(const std::string & sbFilename);

    std::vector<AudioVector> samples;
    void addFromWavs(const std::vector<std::string> &filenames);
    void clearAll();

    void addFromWavList(const std::string &sbFilename) {
        try {
            auto paths = loadRelativePaths(sbFilename);
            addFromWavs(paths);
            std::cout << "SoundBank: Loaded from " << sbFilename << "\n";
        } catch (const std::exception &e) {
            std::cerr << e.what() << "\n";
        }
    }

  private:
    static std::vector<std::string> loadRelativePaths(const std::filesystem::path &listFilePath) {
        std::vector<std::string> result;
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

            // Combine SoundGenerator directory + relative path
            std::filesystem::path fullPath = importFileDir / line;
            result.push_back(fullPath.lexically_normal().string());
        }

        return result;
    }
};