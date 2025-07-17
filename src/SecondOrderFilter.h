#pragma once
#include <cmath>

// Second Order filter: adds a bouncing effect to abrupt value changes (useful to post process rpm)
class SecondOrderFilter {
  public:
    SecondOrderFilter(float frequency, float dampingRatio, float dt)
        : omega_n(2.0f * M_PI * frequency), zeta(dampingRatio), dt(dt), y(0.0f), dy(0.0f), x_prev(0.0f) {}

    float update(float x) {
        float omega2 = omega_n * omega_n;
        float damp = 2.0f * zeta * omega_n;

        float ddy = omega2 * (x - y) - damp * dy;

        dy += ddy * dt;
        y += dy * dt;

        x_prev = x;
        return y;
    }

  private:
    float omega_n;
    float zeta;
    float dt;
    float y;
    float dy;
    float x_prev;
};