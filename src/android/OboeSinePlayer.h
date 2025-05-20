#pragma once

#include <mutex>
#include <oboe/Oboe.h>
#include "AudioContext.h"

class OboeSinePlayer : public oboe::AudioStreamDataCallback {
public:
    explicit OboeSinePlayer(AudioContext* context)
        : mAudioContext(context), mStream(nullptr) {}

    virtual ~OboeSinePlayer() = default;

    int32_t startAudio();
    void stopAudio();

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* oboeStream,
                                          void* audioData,
                                          int32_t numFrames) override;

    //void setAudioContext(AudioContext* context);

private:
    std::mutex mLock;
    oboe::AudioStream* mStream;  // raw pointer
    AudioContext* mAudioContext; // raw pointer

    static constexpr int kChannelCount = 2;
    static constexpr int kSampleRate = 48000;
};
