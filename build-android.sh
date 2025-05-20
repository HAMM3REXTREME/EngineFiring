#!/bin/sh
# Build engineaudio_jni as a .so file for Android

# Set your NDK path
NDK=~/Development/Android/ndk/25.1.8937393

# Create and enter build directory
mkdir -p build-android
cd build-android || exit 1

# Run CMake to configure
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
  -DANDROID_ABI=x86_64 \
  -DANDROID_PLATFORM=android-21 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_SHARED_LIBS=ON

# Build the JNI .so target
cmake --build . --target engineaudio_jni -j$(nproc)
