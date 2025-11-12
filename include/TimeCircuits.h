// TimeCircuits.h
#ifndef TIMECIRCUITS_H
#define TIMECIRCUITS_H

#include "Globals.h"
// Поддержка RTC модуля
#ifdef USE_RTC_DS3231
  #include <RTClib.h>
#endif

class TimeCircuits {
private:
  // Данные времени
  TCDateTime destT;
  TCDateTime presT;
  TCDateTime lastT;
  
  // Буфер дисплея
  byte buf[39];  // DIGITS_TOTAL = 39
  byte curDig;
  unsigned long tDig;
  unsigned long tBlink;
  unsigned long tMin;
  bool blinkTick;
  bool jumpLock;

  #ifdef USE_RTC_DS3231
    RTC_DS3231 rtc;  
    uint8_t lastRTCMinute; // для отслеживания минут модуля
    bool useRTCForDate; // использовать RTC для полной даты (только в диапазоне 2000 - 2099 годов)
  #endif
  
  // Внутренние методы
  void showDigit(byte idx);
  void latchAll(const byte* data);
  void put2(byte off, int value);
  void put4(byte off, int value);
  void putMonth(byte off, int month);
  void fillDash(byte from, byte count);
  void toBufferDest(const TCDateTime& dt);
  void toBufferPres(const TCDateTime& dt);
  void toBufferLast(const TCDateTime& dt);
  void setLeds(const TCDateTime& dt, int pinAM, int pinPM, int pinS1, int pinS2, bool blinkSecs);
  void updatePresentTime();
  void incrementTime(TCDateTime& dt);
  int getDaysInMonth(int month, int year);

public:
  TimeCircuits();
  
  // Инициализация и основной цикл
  void init();
  void update();
  
  // Getters для Destination Time
  TCDateTime getDestTime() const { return destT; }
  bool isDestValid() const { return destT.valid; }
  
  // Setters для Destination Time
  void setDestTime(const TCDateTime& dt);
  void clearDestTime();
  
  // Getters для Present Time
  TCDateTime getPresTime() const { return presT; }
  bool isPresValid() const { return presT.valid; }

  // Setters для Present Time
  void setPresTime(const TCDateTime& dt);
  void clearPresTime();
  
  // Getters для Last Time
  TCDateTime getLastTime() const { return lastT; }
  bool isLastValid() const { return lastT.valid; }
  
  // Setters для Last Time
  void setLastTime(const TCDateTime& dt);
  void clearLastTime();
  
  // Управление прыжками во времени
  bool canTimeTravel() const { return destT.valid && presT.valid && !jumpLock; }
  void timeTravel();
  void unlockJump() { jumpLock = false; }
  bool isJumpLocked() const { return jumpLock; }

  void syncPresTimeFromRTC(); // синхронизируем на время из RTC модуля
  // Обновление дисплея
  void refresh();
};

// Глобальный экземпляр (синглтон)
extern TimeCircuits timeCircuits;

#endif
