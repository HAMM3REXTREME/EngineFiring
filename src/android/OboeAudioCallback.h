#pragma once

#include <memory>
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
    std::shared_ptr<oboe::AudioStream> mStream;
    std::shared_ptr<AudioContext> mMasterAudioContext;

    // Stream params
    static constexpr int kChannelCount = 2;
    static constexpr int kSampleRate = 48000;
};
