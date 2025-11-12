#include <Arduino.h>
#include "TCDateTime.h"
#include "Config.h"
#include "Globals.h"
// временное решение для некоторых переменных, посзднее надо веруть их в main.cpp
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "TimeCircuits.h"

// Логгер
SerialLogger logger;
// Сенсор
TemperatureSensor tempSensor;
// RTC
DS3231RTCProvider rtcProvider;
// Time Circuits
TimeCircuits timeCircuits(&rtcProvider, &logger);


bool isLeapYear(int y) {
  return (y%4==0 && y%100!=0) || (y%400==0);
}

bool isDateValid(int M, int D, int Y, int h, int m) {
  if(M<1 || M>12 || D<1 || h>23 || m>59 || Y<0 || Y>9999) return false;
  int dim[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if(M==2 && isLeapYear(Y)) dim[1] = 29;
  return D <= dim[M-1];
}

TCDateTime parseDateTime(const String& s) {
  TCDateTime d;
  if(s.length() != 12) return d;
  d.m   = s.substring(0,2).toInt();
  d.d   = s.substring(2,4).toInt();
  d.y   = s.substring(4,8).toInt();
  d.h   = s.substring(8,10).toInt();
  d.min = s.substring(10,12).toInt();
  d.valid = isDateValid(d.m, d.d, d.y, d.h, d.min);
  return d;
}

void convertTo12Hour(int h24, int& h12out, bool& pm) {
  pm = (h24 >= 12);
  h12out = h24 % 12;
  if(!h12out) h12out = 12;
}