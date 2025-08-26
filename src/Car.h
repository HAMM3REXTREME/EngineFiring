#ifndef CAR_H
#define CAR_H

#include "Damper.h"
#include <mutex>
#include <queue>

class Car {
  public:
    bool ignition = false; // Ignition

    // Rev limiter
    int revLimiterCutTicks = 2; // Cuts off gas for n ticks if rev limit is reached
    int revLimitTicks = 0;      // ticks with gas cut off remaining
    int revLimit = 8000;        // Gas will be cut when rev limit is reached

    float gearRatios[8] = {0,      0.0820, 0.1362, 0.1879,
                           0.2457, 0.3091, 0.3747, 0.4507}; // Gearing ratios - Used to match engine rpm to wheel rpm (1/(specsheet ratio * final drive))
    float gearDragFactors[8] = {
        0.988,  0.999,  0.9994, 0.9995, 0.9996,
        0.9997, 0.9998, 0.9999}; // Rate for revs to settle back - values closer to 1 need more time to settle back due to less resistance (exponential decay)
    float gearThrottleResponses[8] = {1.2,  0.25, 0.20,  0.15,
                                      0.10, 0.09, 0.082, 0.069}; // Throttle fake sensitivity (or how fast the needle moves) - should feel lower in higher gears

    // Wheel resistances
    double quadraticWheelDrag = 0.999; // Driving drag on wheels (and also engine if in gear)
    float linearWheelDrag = 0;         // Linear drag on wheels (and engine if in gear)

    float clutchKick = 0.1;      // Clutch jerkiness (1 is smooth)
    float boostThreshold = 1500; // Start making fake boost at this rpm

    float wheelRadius = 0.33; // Wheel radius in metres (for wheel speed calculation)

    void tick();

    void setGear(int newGear);
    int getGear();

    void setGas(float newGas);
    float getGas();

    void setRPM(float newRPM); // Sets rpm for next tick
    float getRPM();

    float getBoost();
    void setWheelSpeed(float newSpeed); // Sets wheelRPM for next tick
    float getWheelSpeed();

    float getTorque();

  private:
    std::mutex m_tick;
    float gas = 0;      // Throttle body
    float rpm = 0;      // Engine RPM
    float wheelRPM = 0; // Wheel RPM or speed does not really matter
    float boost = 0;    // Turbo boost
    Damper boostDamper{50};

    float Torque = 0; // Immediate Torque
    int gear = 0;     // Current Gear

    float clutch = 0; // Difference of revs to 'smoothly' join

    Damper rpmDamper{5};
    Damper torqueDamper{1};
    Damper wheelSpeedDamper{10};

    Damper idleValve{5};

    void controlIdle();
    void addEnergy();
};

#endif