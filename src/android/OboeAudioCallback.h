#pragma once

#include <mutex>
#include <oboe/Oboe.h>
#include "AudioContext.h"

class OboeAudioCallback : public oboe::AudioStreamDataCallback {
public:
    explicit OboeAudioCallback(AudioContext* context)
        : mAudioContext(context), mStream(nullptr) {}

    virtual ~OboeAudioCallback() = default;

    int32_t startAudio();
    void stopAudio();

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream* oboeStream,
                                          void* audioData,
                                          int32_t numFrames) override;

    void setAudioContext(AudioContext* context);

private:
    std::mutex mLock;
    oboe::AudioStream* mStream;
    AudioContext* mAudioContext;

    static constexpr int kChannelCount = 2;
    static constexpr int kSampleRate = 48000;
};
