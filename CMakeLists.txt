cmake_minimum_required(VERSION 3.18)
project(EngineAudio LANGUAGES CXX Java)

# -- Needed for JNI native lib --
find_package(Java REQUIRED)
find_package(JNI REQUIRED)
find_package(SWIG REQUIRED)

find_package(PkgConfig REQUIRED)
# -- Needed for desktop program --
pkg_check_modules(SFML REQUIRED sfml-all)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)

include(UseSWIG)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ANDROID_STL_TYPE c++_static)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer")


# C++ source files (not including main.cpp)
set(SRC_FILES
    src/AudioContext.cpp
    src/AudioVector.cpp
    src/BackfireSoundGenerator.cpp
    src/Car.cpp
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
    src
    src/android
)

target_link_libraries(engineaudio_native PRIVATE
    ${SFML_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
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
)

# Enable C++ mode for the interface file BEFORE calling swig_add_library
set_property(SOURCE ${SWIG_INTERFACE_FILE} PROPERTY CPLUSPLUS ON)

# Set SWIG flags BEFORE swig_add_library
set(CMAKE_SWIG_FLAGS -package com.hammer.engine.firing -c++)

swig_add_library(engineaudio_jni
    LANGUAGE java
    SOURCES ${SWIG_INTERFACE_FILE} ${SRC_FILES}
)

if (ANDROID)
    target_include_directories(engineaudio_jni PRIVATE
        oboe/include
    )
    swig_link_libraries(engineaudio_jni
        oboe
        ${JNI_LIBRARIES}
        android
        log
    )
else()
    target_include_directories(engineaudio_jni PRIVATE
        ${PORTAUDIO_INCLUDE_DIRS}
        ${SFML_INCLUDE_DIRS}
    )
    swig_link_libraries(engineaudio_jni
        ${PORTAUDIO_LIBRARIES}
        ${SFML_LIBRARIES}
        ${JNI_LIBRARIES}
    )
endif()
set_target_properties(engineaudio_jni PROPERTIES PREFIX "lib")

# === 3. Optional custom target to build all ===
add_custom_target(all_build
    DEPENDS engineaudio_native engineaudio_jni
)
