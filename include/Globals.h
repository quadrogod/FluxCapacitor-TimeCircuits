#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include "TCDateTime.h"
#include "Config.h"
// временное решение для некоторых переменных, посзднее надо веруть их в main.cpp
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "TimeCircuits.h"

// Логгер
extern SerialLogger logger;
// Сенсор
extern TemperatureSensor tempSensor;
// RTC
extern DS3231RTCProvider rtcProvider;
// Time Circuits
extern TimeCircuits timeCircuits;

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


// ==================== DateTime Structure ====================


// ==================== Time Utilities ====================
bool isLeapYear(int y);
bool isDateValid(int M, int D, int Y, int h, int m);
TCDateTime parseDateTime(const String& s);
void convertTo12Hour(int h24, int& h12out, bool& pm);