# Engine Firing

A Simple C++ program that generates realistic engine audio samples using simple but effective techniques.

## Building

**Make sure you have the following prerequisites:**

- Oboe, Android NDK (For Android only)
- SFML 3 (For C++ desktop program)
- PortAudio (For C++ desktop program)

### Desktop

1) Simply run `./build.sh` to build the C++ program and the JNI shared library.

### Android

1) Edit `./build-android.sh` to point to the NDK path.
2) Run `./build-android.sh` to build the JNI library, the library also requires liboboe.so, which will be built in `build-android/oboe/`.
