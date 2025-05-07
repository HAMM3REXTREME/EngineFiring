// SoundGenerator.h

#pragma once

class SoundGenerator {
  public:
    virtual ~SoundGenerator() = default;
    virtual void update() = 0;
    virtual float getSample() = 0;

    virtual void setAmplitude(float amp) = 0;
    virtual float getAmplitude() const = 0;
};