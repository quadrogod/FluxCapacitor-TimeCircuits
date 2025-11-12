//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_TEMPERATURESENSOR_H
#define FLUXCAPACITOR_TIMECIRCUITS_TEMPERATURESENSOR_H

#include <Arduino.h>
#include "ISensor.h"
#include "Config.h"

class TemperatureSensor : public ISensor {
private:
    const float BETA_THERM = 3950.0;

    uint8_t pin;
    float currentTemp;
    bool animationActive;

    float startThreshold;
    float triggerThreshold;
    float resetThreshold;

public:
    TemperatureSensor(uint8_t ntcPin = NTC_PIN,
                      float start = TEMP_START_THRESHOLD,
                      float trigger = TEMP_FLASH_THRESHOLD,
                      float reset = TEMP_RESET_THRESHOLD)
        : pin(ntcPin),
          currentTemp(0),
          animationActive(false),
          startThreshold(start),
          triggerThreshold(trigger),
          resetThreshold(reset) {
        pinMode(pin, INPUT);
    }

    void setStartThreshold(float value) { startThreshold = value; }
    void setTriggerThreshold(float value) { triggerThreshold = value; }
    void setResetThreshold(float value) { resetThreshold = value; }

    float getStartThreshold() const { return startThreshold; }
    float getTriggerThreshold() const { return triggerThreshold; }
    float getResetThreshold() const { return resetThreshold; }

    float getValue() override {
        return currentTemp;
    }

    float getProgress() override {
        if (currentTemp < startThreshold) return 0.0;
        if (currentTemp >= triggerThreshold) return 1.0;

        float range = triggerThreshold - startThreshold;
        if (range <= 0) return 0.0;

        return (currentTemp - startThreshold) / range;
    }

    bool shouldTrigger() override {
        return currentTemp >= triggerThreshold;
    }

    bool isActive() override {
        return animationActive;
    }

    void update() override {
        int raw = analogRead(pin);
        currentTemp = 1.0 / (log(1.0 / (1023.0 / raw - 1.0)) / BETA_THERM + 1.0 / 298.15) - 273.15;

        if (currentTemp >= startThreshold) {
            animationActive = true;
        } else if (currentTemp < resetThreshold) {
            animationActive = false;
        }
    }
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_TEMPERATURESENSOR_H