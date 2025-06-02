#pragma once

#include "sol/sol.hpp"
#include <vector>
#include <string>
#include <stdexcept>

class LuaBindingHelper {
public:
    // Converts a Lua table to std::vector<int>
    static std::vector<int> toIntVector(const sol::table& table) {
        std::vector<int> vec;
        for (auto& pair : table) {
            vec.push_back(pair.second.as<int>());
        }
        return vec;
    }

    // Converts a Lua table to std::vector<float>
    static std::vector<float> toFloatVector(const sol::table& table) {
        std::vector<float> vec;
        for (auto& pair : table) {
            vec.push_back(pair.second.as<float>());
        }
        return vec;
    }

    // Converts a Lua table to std::vector<std::string>
    static std::vector<std::string> toStringVector(const sol::table& table) {
        std::vector<std::string> vec;
        for (auto& pair : table) {
            vec.push_back(pair.second.as<std::string>());
        }
        return vec;
    }

    // Generic helper with explicit type tag
    template<typename T>
    static std::vector<T> toVector(const sol::table& table);
};

// Template specializations for generic usage
template<>
inline std::vector<int> LuaBindingHelper::toVector<int>(const sol::table& table) {
    return toIntVector(table);
}

template<>
inline std::vector<float> LuaBindingHelper::toVector<float>(const sol::table& table) {
    return toFloatVector(table);
}

template<>
inline std::vector<std::string> LuaBindingHelper::toVector<std::string>(const sol::table& table) {
    return toStringVector(table);
}
