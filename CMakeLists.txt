cmake_minimum_required(VERSION 3.18)
project(EngineAudio LANGUAGES CXX Java)

find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(SWIG REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED sfml-all)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

include(UseSWIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# C++ source files (not including main.cpp)
set(SRC_FILES
    src/SoundGenerator.h
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
)

# === 1. Native executable target (main.cpp : sfml + portaudio) ===
add_executable(engineaudio_native
    src/main.cpp
    ${SRC_FILES}
)

target_include_directories(engineaudio_native PRIVATE
    ${SFML_INCLUDE_DIRS}
    ${PORTAUDIO_INCLUDE_DIRS}
    src
)

target_link_libraries(engineaudio_native PRIVATE
    ${SFML_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
)

# === JNI shared lib ===
set(SWIG_INTERFACE_FILE src/bindings.i)

include_directories(
    ${JNI_INCLUDE_DIRS}
    src
)

# Enable C++ mode for the interface file BEFORE calling swig_add_library
set_property(SOURCE ${SWIG_INTERFACE_FILE} PROPERTY CPLUSPLUS ON)

# Set SWIG flags BEFORE swig_add_library
set(CMAKE_SWIG_FLAGS -package org.engine.firing -c++)

swig_add_library(engineaudio_jni
    LANGUAGE java
    SOURCES ${SWIG_INTERFACE_FILE} ${SRC_FILES}
)

swig_link_libraries(engineaudio_jni
    ${JNI_LIBRARIES}
)

set_target_properties(engineaudio_jni PROPERTIES PREFIX "lib")

# === 3. Optional custom target to build all ===
add_custom_target(all_build
    DEPENDS engineaudio_native engineaudio_jni
)
