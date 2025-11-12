//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_SERIALLOGGER_H
#define FLUXCAPACITOR_TIMECIRCUITS_SERIALLOGGER_H

#include <Arduino.h>
#include "ILogger.h"

class SerialLogger : public ILogger {
public:
    SerialLogger() {}

    void print(const char* message) override {
        Serial.print(message);
    }

    void print(const __FlashStringHelper* message) override {
        Serial.print(message);
    }

    void print(int value) override {
        Serial.print(value);
    }

    void print(float value) override {
        Serial.print(value);
    }

    void println(const char* message) override {
        Serial.println(message);
    }

    void println(const __FlashStringHelper* message) override {
        Serial.println(message);
    }

    void println(int value) override {
        Serial.println(value);
    }

    void println(float value) override {
        Serial.println(value);
    }

    void println() override {
        Serial.println();
    }
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_SERIALLOGGER_H