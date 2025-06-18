cmake_minimum_required(VERSION 3.18)
project(EngineAudio LANGUAGES CXX Java)

# -- ONLY NEEDED FOR JNI NATIVE LIBRARY --
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(SWIG REQUIRED)

# -- ONLY NEEDED FOR DESKTOP PROGRAM --
find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-all)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

# -- USE SYSTEM LUA FOR DESKTOP, MANUALLY IMPORTED FOR ANDROID --
if (ANDROID)
    # Use manually imported Lua static lib for Android
    add_library(lua STATIC IMPORTED)

    set_target_properties(lua PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lua/lib/${ANDROID_ABI}/liblua.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/lua/include
    )

    set(LUA_LIBRARIES lua)
    set(LUA_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/lua/include)
else()
    # Use system-installed Lua on desktop (via pkg-config)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LUA REQUIRED lua)

    set(LUA_LIBRARIES ${LUA_LIBRARIES})
    set(LUA_INCLUDE_DIRS ${LUA_INCLUDE_DIRS})
endif()


include(UseSWIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_STL_TYPE c++_static)

# C++ source files (not including main.cpp)
set(SRC_FILES
    src/AudioContext.cpp
    src/AudioVector.cpp
    src/BackfireSoundGenerator.cpp
    src/Car.cpp
    src/Damper.cpp
    src/Engine.cpp
    src/EngineSoundGenerator.cpp
    src/SimpleSoundGenerator.cpp
    src/SoundBank.cpp
    src/TurboWhooshGenerator.cpp
    src/Biquad.cpp
)

# Detect Android and add Oboe-specific sources + build Oboe
if (ANDROID)
    set(IS_ANDROID TRUE)
    message(STATUS "Targeting Android")

    list(APPEND SRC_FILES src/android/OboeAudioCallback.cpp)

    add_subdirectory(oboe)

    add_definitions(-DUSE_OBOE)
endif()

# === 1. Native executable target (main.cpp : sfml + portaudio) ===
add_executable(engineaudio_native
    src/main.cpp
    ${SRC_FILES}
)

target_include_directories(engineaudio_native PRIVATE
    ${SFML_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIRS}
    src
    src/sol
)

target_link_libraries(engineaudio_native PRIVATE
    ${SFML_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${LUA_LIBRARIES} 
)

# === JNI shared lib ===
if (ANDROID)
    set(SWIG_INTERFACE_FILE src/bindings-android.i)
else()
    set(SWIG_INTERFACE_FILE src/bindings.i)
endif()

include_directories(
    ${JNI_INCLUDE_DIRS}
    src
    src/sol
)

# Enable C++ mode for the interface file BEFORE calling swig_add_library
set_property(SOURCE ${SWIG_INTERFACE_FILE} PROPERTY CPLUSPLUS ON)

# Set SWIG flags BEFORE swig_add_library
set(CMAKE_SWIG_FLAGS -package engine.firing -c++)

swig_add_library(engineaudio_jni
    LANGUAGE java
    SOURCES ${SWIG_INTERFACE_FILE} ${SRC_FILES}
)

if (ANDROID)
    target_include_directories(engineaudio_jni PRIVATE
        oboe/include
        ${JNI_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIRS}
        src
        src/sol
    )
    swig_link_libraries(engineaudio_jni
        oboe
        ${JNI_LIBRARIES}
        ${LUA_LIBRARIES}
        android
        log
    )
else()
    target_include_directories(engineaudio_jni PRIVATE
        ${JNI_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIRS}
        src
        src/sol
    )
    swig_link_libraries(engineaudio_jni
        ${JNI_LIBRARIES}
        ${LUA_LIBRARIES}
    )
endif()
set_target_properties(engineaudio_jni PROPERTIES PREFIX "lib")

# === 3. Optional custom target to build all ===
add_custom_target(all_build
    DEPENDS engineaudio_native engineaudio_jni
)
