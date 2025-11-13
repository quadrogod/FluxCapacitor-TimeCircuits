//
// Created by Alexey Molchanov on 13.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_STUBSENSOR_H
#define FLUXCAPACITOR_TIMECIRCUITS_STUBSENSOR_H

#include <Arduino.h>
#include "ISensor.h"

class StubSensor : public ISensor {
private:
    float stubValue;

public:
    explicit StubSensor(float value = 25.0)
        : stubValue(value) {}

    float getValue() override {
        return stubValue;
    }

    float getProgress() override {
        return 0.0;
    }

    bool shouldTrigger() override {
        return false;
    }

    bool isActive() override {
        return false;
    }

    void update() override {
    }
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_STUBSENSOR_H