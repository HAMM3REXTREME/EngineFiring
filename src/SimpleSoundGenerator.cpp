#include "SimpleSoundGenerator.h"
#include <algorithm>
#include <iostream>
#include <sstream>

SimpleSoundGenerator::SimpleSoundGenerator(SoundBank &bank) : soundBank(bank), currentSampleIndex(-1), playbackPosition(0), gain(1.0f) {}

// Call this to start playing a sample by its index in the SoundBank
void SimpleSoundGenerator::startPlayback(size_t sampleIndex, bool loop) {
    if (sampleIndex < soundBank.samples.size()) {
        currentSampleIndex = static_cast<int>(sampleIndex);
        playbackPosition = 0;
        looping = loop;
    }
}
void SimpleSoundGenerator::stopPlayback() {
    currentSampleIndex = -1;
    playbackPosition = 0;
}
void SimpleSoundGenerator::update() {
    // Just advances the playback position if a sample is active
    if (isPlaying()) {
        ++playbackPosition;
        if (playbackPosition >= soundBank.samples[currentSampleIndex].samples.size()) {
            if (looping) {
                playbackPosition = 0; // Reset to start of sample for looping
            } else {
                currentSampleIndex = -1; // Stop playback if not looping
            }
        }
    }
}
float SimpleSoundGenerator::getSample() {
    if (isPlaying()) {
        return gain * soundBank.samples[currentSampleIndex].samples[playbackPosition];
    }
    return 0.0f; // Silence when not playing
}

std::string SimpleSoundGenerator::getInfo(int depth) const {
    std::ostringstream oss;
    for (int i = 0; i < depth; ++i) {
        oss << "    ";
    }
    oss << "SimpleSoundGenerator: gain " << gain << ", looping " << looping << ", playing sample " << currentSampleIndex << ", playback position " << playbackPosition << "\n";
    return oss.str();
}

void SimpleSoundGenerator::setAmplitude(float newGain) { gain = std::fmax(0.0f, newGain); }
float SimpleSoundGenerator::getAmplitude() const { return gain; }

bool SimpleSoundGenerator::isPlaying() const {
    return currentSampleIndex >= 0 && currentSampleIndex < static_cast<int>(soundBank.samples.size()) &&
           (looping || playbackPosition < soundBank.samples[currentSampleIndex].samples.size());
}