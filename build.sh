#!/bin/sh
# Build the program
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)  engineaudio_native # Automatically use all CPU cores
rsync -a --delete ../assets/ assets/
