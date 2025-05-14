#ifndef CAR_H
#define CAR_H

#include "Damper.h"
#include <mutex>
#include <queue>

class Car {
  public:
    bool ignition = false; // Ignition

    // Rev limiter
    int defaultRevLimitTick = 10; // Cuts off gas for n ticks if rev limit is reached.
    int revLimitTick = 0;         // no-gas ticks remaining
    int revLimit = 8000;          // Gas will be cut when rev limit is reached

    float gearRatios[8] = {0, 1, 1.4, 2, 2.75, 3.85, 5.4, 7.5}; // Gearing ratios - Used to match engine rpm to wheel rpm
    float gearLazyValues[8] = {0.99,   0.999,  0.9994, 0.9995, 0.9996,
                               0.9997, 0.9998, 0.9999}; // Engine time for revs to settle back - values closer to one need more time to go back to idle cause
                                                        // less resistance (exponential decay)
    float gearThrottleResponses[8] = {
        1, 0.25, 0.20, 0.15, 0.10, 0.09, 0.082, 0.069}; // Throttle sensitivity - Should feel lower in higher gears since the high gears is hard on the engine.

    // Wheel resistances
    double quadraticWheelDrag = 0.999; // Driving drag on wheels (and also engine if in gear)
    float linearWheelDrag = 0;         // Linear drag on wheels (and engine if in gear)

    float clutchKick = 0.6; // Clutch jerkiness (1 is smooth)
    float boostThreshold = 1500;

    std::mutex m_tick;

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
    float gas = 0;      // Throttle body
    float rpm = 0;      // Engine RPM
    float wheelRPM = 0; // Wheel RPM or speed does not really matter
    float boost = 0;    // Turbo boost
    Damper boostDamper{50};

    float idleValve = 1; // Idle valve
    float Torque = 0;    // Immediate Torque
    int gear = 0;        // Current Gear

    float clutch = 0; // Difference of revs to 'smoothly' join

    Damper rpmDamper{5};
    Damper wheelSpeedDamper{10};

    void controlIdle();
    void addEnergy();
};

#endif