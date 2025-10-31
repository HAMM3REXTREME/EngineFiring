#pragma once
#include <atomic>
#include <cmath>

class Damper {
public:
    // dampingFactor in range [0, 1): smaller = more damping (slower)
    // Example: 0.1 = heavy damping, 0.8 = light damping
    explicit Damper(double dampingFactor = 0.2)
        : damping(dampingFactor), value(0.0), initialized(false) {}

    // Add a new input value (thread-safe)
    void addValue(double newValue) {
        double current = value.load(std::memory_order_relaxed);

        if (!initialized) {
            value.store(newValue, std::memory_order_relaxed);
            initialized = true;
            return;
        }

        double smoothed = current + damping * (newValue - current);
        value.store(smoothed, std::memory_order_relaxed);
    }

    // Get the current damped value (thread-safe)
    double getAverage() const {
        return value.load(std::memory_order_relaxed);
    }

private:
    const double damping;
    std::atomic<double> value;
    bool initialized;
};
