#include "TurboWhooshGenerator.h"

    TurboWhooshGenerator::TurboWhooshGenerator(float sampleRate)
        : sampleRate(sampleRate), intensity(0.0f), phase(0.0f), amplitude(0.5f) // Default gain set to 0.5f
    {
        resetFilter();
        srand(static_cast<unsigned>(time(nullptr)));
    }

void TurboWhooshGenerator::update() {} // Nothing needed

        float TurboWhooshGenerator::getSample(){
        // Base frequencies for high-pitched whistle and low rumble
        float highFreq = 1200.0f + intensity * 3000.0f; // Whistle pitch increases with boost
        float lowFreq = 80.0f + intensity * 300.0f;     // Rumble pitch for the low end

        // Filter sharpness based on intensity
        float highQ = 2.0f + intensity * 4.0f; // Whistle sharper at higher boost
        float lowQ = 0.5f + intensity * 2.0f;  // Rumble smoother at low boost

        // Overall gain adjusted by the instance gain variable
        float effectiveGain = amplitude * (0.2f + intensity * 0.8f);

        // High-frequency filter for the whistle
        computeBandpassCoeffs(highFreq, highQ);
        float highNoise = whiteNoise();
        float highFiltered = processBandpass(highNoise);

        // Low-frequency filter for the whoosh
        computeBandpassCoeffs(lowFreq, lowQ);
        float lowNoise = whiteNoise();
        float lowFiltered = processBandpass(lowNoise);

        // Combine high and low components to create the full turbo sound
        return effectiveGain * (highFiltered + lowFiltered);
    }


        void TurboWhooshGenerator::setIntensity(float newIntensity) {
        // Smooth intensity updates for no stepping
        float clamped = std::fmax(0.0f, std::fmin(1.0f, newIntensity));
        intensity = 0.9f * intensity + 0.1f * clamped; // Smooth intensity change
    }

    float TurboWhooshGenerator::getIntensity() const { return intensity; }

        // Set the gain for overall volume control
    void TurboWhooshGenerator::setAmplitude(float newGain) {
        amplitude = std::fmax(0.0f, newGain); // Ensure gain is not negative
    }

        // Get the current gain value
    float TurboWhooshGenerator::getAmplitude() const { return amplitude; }

        void TurboWhooshGenerator::resetFilter() { x1 = x2 = y1 = y2 = 0.0f; }

    float TurboWhooshGenerator::whiteNoise() { return 2.0f * ((float)rand() / (float)RAND_MAX) - 1.0f; }

    void TurboWhooshGenerator::computeBandpassCoeffs(float freq, float Q) {
        float omega = 2.0f * M_PI * freq / sampleRate;
        float alpha = sin(omega) / (2.0f * Q);

        float cos_omega = cos(omega);
        float norm = 1.0f / (1.0f + alpha);

        a0 = alpha * norm;
        a1 = 0.0f;
        a2 = -alpha * norm;
        b1 = -2.0f * cos_omega * norm;
        b2 = (1.0f - alpha) * norm;
    }

    float TurboWhooshGenerator::processBandpass(float input) {
        float output = a0 * input + a1 * x1 + a2 * x2 - b1 * y1 - b2 * y2;
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        return output;
    }