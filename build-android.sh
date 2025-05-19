#!/bin/sh
# Build as an .so file for Android
mkdir -p build-android
cd build-android
cmake ..   -DCMAKE_TOOLCHAIN_FILE=~/Development/Android/ndk/29.0.13113456/build/cmake/android.toolchain.cmake   -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-21   -DCMAKE_BUILD_TYPE=Release   -DBUILD_SHARED_LIBS=ON
cmake --build . --target engineaudio_jni -j$(nproc)
