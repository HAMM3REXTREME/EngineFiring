# Engine Firing

A Simple C++ program (and JNI library) that generates realistic engine audio samples using simple but effective techniques.

## Building

**Make sure you have the following prerequisites:**

- A modern C++ compiler + CMake
- SWIG (For JNI shared library)
- Android NDK (For Android JNI lib)
- SFML 3 (For C++ desktop program)
- PortAudio (For C++ desktop program)
- Lua (for making engines with unique behaviours at runtime)

```bash
git clone --recursive https://github.com/HAMM3REXTREME/EngineFiring
cd EngineFiring
```

### Desktop

1) Simply run `./build.sh` to build the C++ program and the JNI shared library.

### Android

1) Build Lua for Android (using the NDK), place the resulting `liblua.a` file inside `lua/lib/{ANDROID_ABI}/`.
2) Edit `./build-android.sh` to point to your Android NDK installation path, optionally edit the target ABI.
3) Run `./build-android.sh` to build the JNI library, the library also requires liboboe.so, which will be built in `build-android/oboe/`.
4) Place both `.so` files inside `app/src/main/jniLibs/{ANDROID_ABI}/` inside your Android project directory. Also copy the generated java files into your app.
