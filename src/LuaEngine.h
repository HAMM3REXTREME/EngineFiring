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

class LuaEngine {
  public:
    sol::state lua;
    sol::function tickFunction;
    int runLuaScript(const std::string &filename) {
        try {
            lua.script_file(filename);
        } catch (const sol::error &e) {
            std::cerr << "Lua error: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }
    void tick() {
        sol::protected_function_result pfr = tickFunction();
        if (!pfr.valid()) {
            sol::error err = pfr;
            std::cerr << "Error: " << err.what() << std::endl;
        }
    }
    LuaEngine() {
        lua.open_libraries(sol::lib::base);
        lua["vecFloat"] = &tableToVectorFloat;
        lua["vecInt"] = &tableToVectorInt;
        tickFunction = lua["tick"];
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
};