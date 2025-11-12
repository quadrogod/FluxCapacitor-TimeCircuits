#pragma once

#include <FastLED.h>
#include "Config.h"

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

#pragma once
#include <Arduino.h>

// ==================== DateTime Structure ====================
struct TCDateTime {
  int m, d, y, h, min;
  bool valid;

  TCDateTime() { m=d=y=h=min=0; valid=false; }

  String toText() const {
    char buf[20];
    snprintf(buf, sizeof(buf), "%02d.%02d.%04d %02d:%02d", d, m, y, h, min);
    return String(buf);
  }
};

// ==================== Time Utilities ====================
bool isLeapYear(int y);
bool isDateValid(int M, int D, int Y, int h, int m);
TCDateTime parseDateTime(const String& s);
void convertTo12Hour(int h24, int& h12out, bool& pm);