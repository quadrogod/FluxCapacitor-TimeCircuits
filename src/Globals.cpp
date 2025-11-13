#include "Globals.h"
// временное решение для некоторых переменных, позднее надо веруть их в main.cpp
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "TimeCircuits.h"
#include "AnimationManager.h"

// Логгер
SerialLogger logger;
// Сенсор
TemperatureSensor tempSensor;
// RTC
DS3231RTCProvider rtcProvider;
// Time Circuits
TimeCircuits timeCircuits(&rtcProvider, &logger);

AnimationManager animationManager(&tempSensor, &logger);

void setupComponentLinks() {
    animationManager.setTimeTravelValidator(&timeCircuits);
}
