#include "OboeAudioCallback.h"

using namespace oboe;

int32_t OboeAudioCallback::startAudio() {
    std::lock_guard<std::mutex> lock(mLock);

    AudioStreamBuilder builder;
    Result result = builder.setSharingMode(SharingMode::Exclusive)
        ->setPerformanceMode(PerformanceMode::LowLatency)
        ->setChannelCount(kChannelCount)
        ->setSampleRate(kSampleRate)
        ->setSampleRateConversionQuality(SampleRateConversionQuality::Medium)
        ->setFormat(AudioFormat::Float)
        ->setDataCallback(this)
        ->openStream(&mStream);

    if (result != Result::OK) return static_cast<int32_t>(result);

    return static_cast<int32_t>(mStream->requestStart());
}

void OboeAudioCallback::stopAudio() {
    std::lock_guard<std::mutex> lock(mLock);
    if (mStream) {
        mStream->stop();
        mStream->close();
        delete mStream;
        mStream = nullptr;
    }
}

void OboeAudioCallback::setAudioContext(AudioContext* context){
    std::lock_guard<std::mutex> lock(mLock);
    mAudioContext = context;
}

DataCallbackResult OboeAudioCallback::onAudioReady(AudioStream* oboeStream, void* audioData, int32_t numFrames) {
    float* floatData = static_cast<float*>(audioData);

    if (mAudioContext) {
        mAudioContext->getAllSamples(floatData, numFrames, kChannelCount);
    } else {
        // Fill with silence if context not available
        std::fill(floatData, floatData + numFrames * kChannelCount, 0.0f);
    }

    return DataCallbackResult::Continue;
}
