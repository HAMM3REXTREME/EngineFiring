#pragma once
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

class SimpleConfig {
    std::unordered_map<std::string, std::string> data;

    static std::string trim(const std::string &s) {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])))
            ++start;

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])))
            --end;

        return s.substr(start, end - start);
    }

  public:
    bool load(const std::string &path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Error: Failed to open config file: " << path << "\n";
            return false;
        }

        std::string line;
        int lineNum = 0;
        while (std::getline(file, line)) {
            ++lineNum;

            // remove comments
            auto commentPos = line.find('#');
            if (commentPos != std::string::npos)
                line = line.substr(0, commentPos);

            line = trim(line);
            if (line.empty())
                continue;

            std::istringstream iss(line);
            std::string key, value;
            if (!(iss >> key >> value)) {
                std::cerr << "Warning: Invalid line " << lineNum << " in " << path << ": '" << line << "'\n";
                continue;
            }

            key = trim(key);
            value = trim(value);

            data[key] = value;
        }

        return true;
    }

    std::string getString(const std::string &key, const std::string &defaultValue = "") const {
        auto it = data.find(key);
        return it != data.end() ? it->second : defaultValue;
    }

    int getInt(const std::string &key, int defaultValue = 0) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::stoi(it->second);
            } catch (const std::exception &e) {
                std::cerr << "Error: Invalid integer for key '" << key << "': " << it->second << "\n";
            }
        }
        return defaultValue;
    }

    float getFloat(const std::string &key, float defaultValue = 0.0f) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::stof(it->second);
            } catch (const std::exception &e) {
                std::cerr << "Error: Invalid float for key '" << key << "': " << it->second << "\n";
            }
        }
        return defaultValue;
    }

    bool getBool(const std::string &key, bool defaultValue = false) const {
        auto it = data.find(key);
        if (it != data.end()) {
            std::string val = it->second;
            return val == "1" || val == "true" || val == "True";
        }
        return defaultValue;
    }
    bool hasKey(const std::string &key) const { return data.find(key) != data.end(); }
};
