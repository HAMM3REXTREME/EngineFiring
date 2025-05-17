// Simplified car simulator - not physically accurate

#include "Car.h"
#include "Damper.h"

#include <mutex>

void Car::tick() {
    std::lock_guard<std::mutex> lock(m_tick);
    controlIdle();
    rpm = rpm * gearLazyValues[gear]; // Apply engine internal drag
    addEnergy();

    // Apply clutch revs in multiple smaller chunks of revs, makes a kick, like dumping the clutch in a real car
    rpm += clutch * clutchKick;
    clutch = (1.0 - clutchKick) * clutch;

    // Set Wheel RPM depending on the engine rpm, current gear ratio and coasting drag
    if (gear >= 1) {
        wheelRPM = rpm * gearRatios[gear] * quadraticWheelDrag - linearWheelDrag;
        rpm = rpm * quadraticWheelDrag - (linearWheelDrag / gearRatios[gear]);
    } else {
        // Just apply the rolling and brake resistance if in neutral
        wheelRPM = wheelRPM * quadraticWheelDrag - linearWheelDrag;
    }
    rpm < 0 ? rpm = 0 : 0;
    wheelRPM < 0 ? wheelRPM = 0 : 0;

    rpmDamper.addValue(rpm);
    wheelSpeedDamper.addValue(wheelRPM);
}

void Car::setGear(int newGear) {
    gear = newGear;
    if (gear <= 0) {
        return;
    }
    if (wheelRPM <= 0 && rpm >= 700) { // 'Dumping' the clutch won't stall
        wheelRPM = rpm / 7;
    }
    clutch = wheelRPM / gearRatios[gear] - rpm;
}

void Car::controlIdle() {
    if (rpm >= 801) { // Idle air control valve
        idleValve = 5;
    } else if (rpm <= 795) {
        idleValve = 10;
    }
}

void Car::addEnergy() {
    if (rpm > 50) {
        rpm += Torque; // Don't divide by zero
        if (ignition) {
            if (rpm <= revLimit) { // Rev limiter thingy
                if (revLimitTick <= 0) {
                    Torque = (gas + idleValve) * gearThrottleResponses[gear];
                } else {
                    revLimitTick--;
                }
            } else {
                // Start a rev limit cut cycle if rpm exceeds the limit
                revLimitTick = defaultRevLimitTick;
                Torque = 0;
            }

        } else {
            Torque = 0;
        }
    }
    if (getRPM() >= boostThreshold) {
        boostDamper.addValue(getTorque() * getRPM() / 8000);
    } else {
        boostDamper.addValue(0);
    }
}

int Car::getGear() { return gear; }

void Car::setGas(float newGas) { gas = newGas; }
float Car::getGas() { return gas; }

void Car::setRPM(float newRPM) { rpm = newRPM; } // Sets rpm for next tick
float Car::getRPM() { return rpmDamper.getAverage(); }

float Car::getBoost() { return boostDamper.getAverage(); }

void Car::setWheelSpeed(float newSpeed) { wheelRPM = newSpeed; } // Sets wheelRPM for next tick
float Car::getWheelSpeed() { return wheelSpeedDamper.getAverage(); }

float Car::getTorque() { return Torque / gearThrottleResponses[gear]; }
