#pragma once

#include <FastLED.h>
#include "Config.h"
// временное решение для некоторых переменных, посзднее надо веруть их в main.cpp
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "TimeCircuits.h"
#include "AnimationManager.h"

// Логгер
extern SerialLogger logger;
// Сенсор
extern TemperatureSensor tempSensor;
// RTC
extern DS3231RTCProvider rtcProvider;
// Time Circuits
extern TimeCircuits timeCircuits;

extern AnimationManager animationManager;

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
