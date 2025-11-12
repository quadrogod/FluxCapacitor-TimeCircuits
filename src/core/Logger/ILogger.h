//
// Created by Alexey Molchanov on 12.11.2025.
//
// Разнес логику, что бы можно было заменить отображение на дисплее, в консоле или вообще записать в файл
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_ILOGGER_H
#define FLUXCAPACITOR_TIMECIRCUITS_ILOGGER_H

#include <Arduino.h>

class ILogger {
public:
    virtual ~ILogger() {}

    virtual void print(const char* message) = 0;
    virtual void print(const __FlashStringHelper* message) = 0;
    virtual void print(int value) = 0;
    virtual void print(float value) = 0;
    virtual void print(String message) = 0;

    virtual void println(const char* message) = 0;
    virtual void println(const __FlashStringHelper* message) = 0;
    virtual void println(int value) = 0;
    virtual void println(float value) = 0;
    virtual void println(String message) = 0;
    virtual void println() = 0;
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_ILOGGER_H