// Simplified car simulator - not physically accurate

#include "Car.h"
#include "Damper.h"
#include <math.h>

#include <mutex>

void Car::tick() {
    std::lock_guard<std::mutex> lock(m_tick);
    controlIdle();
    rpm = rpm * gearDragFactors[gear]; // Apply engine internal drag
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
    wheelSpeedDamper.addValue(wheelRPM * 2.0 * M_PI * wheelRadius * (1.0 / 60.0));
}

void Car::setGear(int newGear) {
    gear = newGear;
    if (gear <= 0) {
        return;
    }
    if (wheelRPM <= 5 && rpm >= 700) { // 'Dumping' the clutch won't stall
        wheelRPM = rpm * gearRatios[gear] / 8;
        clutchKick = 0.05;
    } else {
        clutchKick = 0.4;
    }
    clutch = wheelRPM / gearRatios[gear] - rpm;
}

void Car::controlIdle() {
    if (rpm >= 905) { // Idle air control valve
        idleValve.addValue(8);
    } else if (rpm <= 900) {
        idleValve.addValue(0.02 * (895 - rpm) * gearRatios[gear] + 10);
    }
}

void Car::addEnergy() {
    if (rpm > 50) {
        rpm += Torque * gearThrottleResponses[gear] * (1 + boostDamper.getAverage() / 350);
        if (ignition) {
            // Rev limiter thingy
            if (rpm <= revLimit) {
                if (revLimitTicks <= 0) {
                    Torque = (gas + idleValve.getAverage());
                } else {
                    revLimitTicks--;
                }
            } else {
                // Start a rev limit cut cycle if rpm exceeds the limit
                revLimitTicks = revLimiterCutTicks;
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
    torqueDamper.addValue(Torque);
}

int Car::getGear() { return gear; }

void Car::setGas(float newGas) { gas = newGas; }
float Car::getGas() { return gas; }

void Car::setRPM(float newRPM) { rpm = newRPM; } // Sets rpm for next tick
float Car::getRPM() { return rpmDamper.getAverage(); }

float Car::getBoost() { return boostDamper.getAverage(); }

void Car::setWheelSpeed(float newSpeed) { wheelRPM = newSpeed; } // Sets wheelRPM for next tick
float Car::getWheelSpeed() { return wheelSpeedDamper.getAverage(); }

float Car::getTorque() { return torqueDamper.getAverage(); }
