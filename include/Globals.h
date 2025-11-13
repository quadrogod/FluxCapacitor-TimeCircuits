#pragma once

#include <FastLED.h>
#include "Config.h"
// временное решение для некоторых переменных, посзднее надо веруть их в main.cpp
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/Sensor/StubSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "TimeCircuits.h"
#include "AnimationManager.h"

// Логгер
extern SerialLogger logger;
// Сенсор
#if USE_STUB_SENSOR
    extern StubSensor sensor;
    // Serial.println("Using STUB sensor (25°C)");
#else
    extern TemperatureSensor sensor;
    // Serial.println("Using REAL temperature sensor");
#endif
// extern TemperatureSensor sensor;
// RTC
extern DS3231RTCProvider rtcProvider;
// Time Circuits
extern TimeCircuits timeCircuits;

extern AnimationManager animationManager;

void setupComponentLinks();

/********************************************************************
 *  GLOBAL VARIABLES
 ********************************************************************/
extern CRGB leds[NUM_LEDS];

extern uint8_t hue;
extern int delaySpeed;
extern float movieSpeed;

// Animation mode flags
extern bool timeTravel;
extern bool smoothChase;
extern bool movieChase;
extern bool movieChaseSimple;
extern bool thirtyChase;
extern bool radChase;
extern bool radChase2;
extern bool rainbowChase;

// Timer
extern unsigned long previousTime;
