#!/bin/sh
# Build the program
mkdir -p build
cd build
cmake ..
make -j$(nproc)  engineaudio_native # Automatically use all CPU cores
rsync -a --delete ../assets/ assets/
