// TimeCircuits.cpp
#include <Arduino.h>
#include "Config.h"
#include "Globals.h"
#include "TimeCircuits.h"
#ifdef USE_RTC_DS3231
  // сам модуль для установки даты использовать не буду, т.к. он ограничен 2000-м годом, буду устанавливать в нем дату, например 01.01.2020 00:00:00 и от неё считать тики модуля
  #include <RTClib.h>
#endif

/* ==================== Segment Patterns ==================== */
enum {D0,D1,D2,D3,D4,D5,D6,D7,D8,D9,D_MINUS,D_BLANK,
      D_A,D_b,D_C,D_d,D_E,D_F,D_G,D_H,D_J,D_L,D_n,D_o,D_P,D_r,D_S,D_t,D_U,D_y};

static const byte SEG_PAT[] = {
  0b11000000, // 0
  0b11111001, // 1
  0b10100100, // 2
  0b10110000, // 3
  0b10011001, // 4
  0b10010010, // 5
  0b10000011, // 6
  0b11111000, // 7
  0b10000000, // 8
  0b10011000, // 9
  0b10111111, // - (минус)
  0b11111111, // blank (пусто)
  // Буквы для месяцев
  0b10001000, // A
  0b10000011, // b
  0b11000110, // C
  0b10100001, // d
  0b10000110, // E
  0b10001110, // F
  0b11000010, // G
  0b10001001, // H
  0b11100001, // J
  0b11000111, // L
  0b10101011, // n
  0b10100011, // o
  0b10001100, // P
  0b10101111, // r
  0b10010010, // S
  0b10000111, // t
  0b11000001, // U
  0b10010001, // y
};

const byte MONTH_LETTERS[12][3] = {
  {D_J, D_A, D_n}, {D_F, D_E, D_b}, {D_H, D_A, D_r},
  {D_A, D_P, D_r}, {D_H, D_A, D_y}, {D_J, D_U, D_n},
  {D_J, D_U, D_L}, {D_A, D_U, D_G}, {D_S, D_E, D_P},
  {D_o, D_C, D_t}, {D_n, D_o, D_BLANK}, {D_d, D_E, D_C}
};

/* ==================== Digit Selection Table ==================== */
struct Sel { byte srIdx; byte bitMask; };
static const Sel SEL[39] PROGMEM = {
  // Destination: 0-12
  {0,1<<0},{0,1<<1},{0,1<<2},{0,1<<3},{0,1<<4},
  {0,1<<5},{0,1<<6},{1,1<<0},{1,1<<1},{1,1<<2},
  {1,1<<3},{1,1<<4},{1,1<<5},
  // Present: 13-25
  {1,1<<6},{1,1<<7},{2,1<<0},{2,1<<1},{2,1<<2},
  {2,1<<3},{2,1<<4},{2,1<<5},{2,1<<6},{2,1<<7},
  {3,1<<0},{3,1<<1},{3,1<<2},
  // Last: 26-38
  {3,1<<3},{3,1<<4},{3,1<<5},{3,1<<6},{3,1<<7},
  {4,1<<0},{4,1<<1},{4,1<<2},{4,1<<3},{4,1<<4},
  {4,1<<5},{4,1<<6},{4,1<<7}
};

/* ==================== Constructor ==================== */
TimeCircuits::TimeCircuits() 
  : curDig(0), tDig(0), tBlink(0), tMin(0), blinkTick(false), jumpLock(false) 
  #ifdef USE_RTC_DS3231
    , lastRTCMinute(255), useRTCForDate(false) // 255 = неинициализировано
  #endif
{
  memset(buf, D_BLANK, sizeof(buf));
}

/* ==================== Hardware Functions ==================== */
void TimeCircuits::latchAll(const byte* data) {
  digitalWrite(TC_LATCH_PIN, LOW);
  for (int8_t i = SR_CNT-1; i >= 0; --i) {
    shiftOut(TC_DATA_PIN, TC_CLOCK_PIN, MSBFIRST, data[i]);
  }
  digitalWrite(TC_LATCH_PIN, HIGH);
}

void TimeCircuits::showDigit(byte idx) {
  static const byte CLR[SR_CNT] = {0};
  byte sr[SR_CNT] = {0};
  
  byte sym = buf[idx];
  if (sym >= sizeof(SEG_PAT)) sym = D_BLANK;
  sr[0] = SEG_PAT[sym];
  
  Sel sel;
  memcpy_P(&sel, &SEL[idx], sizeof(Sel));
  sr[1 + sel.srIdx] = sel.bitMask;
  
  latchAll(sr);
  delayMicroseconds(DIGIT_US);
  latchAll(CLR);
}

/* ==================== Buffer Fill Functions ==================== */
void TimeCircuits::put2(byte off, int value) {
  buf[off] = (value / 10) % 10;
  buf[off + 1] = value % 10;
}

void TimeCircuits::put4(byte off, int value) {
  buf[off] = (value / 1000) % 10;
  buf[off + 1] = (value / 100) % 10;
  buf[off + 2] = (value / 10) % 10;
  buf[off + 3] = value % 10;
}

void TimeCircuits::putMonth(byte off, int month) {
  if (month < 1 || month > 12) {
    buf[off] = buf[off+1] = buf[off+2] = D_MINUS;
    return;
  }
  buf[off] = MONTH_LETTERS[month-1][0];
  buf[off+1] = MONTH_LETTERS[month-1][1];
  buf[off+2] = MONTH_LETTERS[month-1][2];
}

void TimeCircuits::fillDash(byte from, byte count) {
  for (byte i = 0; i < count; i++) buf[from + i] = D_MINUS;
}

/* ==================== DateTime to Buffer ==================== */
void TimeCircuits::toBufferDest(const TCDateTime& dt) {
  if (!dt.valid) {
    fillDash(0, 13);
    return;
  }
  putMonth(0, dt.m);
  put2(3, dt.d);
  put4(5, dt.y);
  int h12; bool pm;
  convertTo12Hour(dt.h, h12, pm);
  put2(9, h12);
  put2(11, dt.min);
}

void TimeCircuits::toBufferPres(const TCDateTime& dt) {
  if (!dt.valid) {
    fillDash(13, 13);
    return;
  }
  putMonth(13, dt.m);
  put2(16, dt.d);
  put4(18, dt.y);
  int h12; bool pm;
  convertTo12Hour(dt.h, h12, pm);
  put2(22, h12);
  put2(24, dt.min);
}

void TimeCircuits::toBufferLast(const TCDateTime& dt) {
  if (!dt.valid) {
    fillDash(26, 13);
    return;
  }
  putMonth(26, dt.m);
  put2(29, dt.d);
  put4(31, dt.y);
  int h12; bool pm;
  convertTo12Hour(dt.h, h12, pm);
  put2(35, h12);
  put2(37, dt.min);
}

/* ==================== LED Control ==================== */
void TimeCircuits::setLeds(const TCDateTime& dt, int pinAM, int pinPM, int pinS1, int pinS2, bool blinkSecs) {
  if (!dt.valid) {
    digitalWrite(pinAM, LOW);
    digitalWrite(pinPM, LOW);
    digitalWrite(pinS1, LOW);
    digitalWrite(pinS2, LOW);
    return;
  }
  
  bool pm = (dt.h >= 12);
  digitalWrite(pinAM, pm ? LOW : HIGH);
  digitalWrite(pinPM, pm ? HIGH : LOW);
  
  if (blinkSecs) {
    digitalWrite(pinS1, blinkTick ? HIGH : LOW);
    digitalWrite(pinS2, blinkTick ? HIGH : LOW);
  } else {
    digitalWrite(pinS1, HIGH);
    digitalWrite(pinS2, HIGH);
  }
}

/* ==================== Time Increment with Full Date Logic ==================== */
void TimeCircuits::incrementTime(TCDateTime& dt) {
  if (!dt.valid) return;
  
  // Увеличиваем минуты
  dt.min++;
  
  // Проверка переполнения минут -> час
  if (dt.min >= 60) {
    dt.min = 0;
    dt.h++;
    
    // Проверка переполнения часов -> день
    if (dt.h >= 24) {
      dt.h = 0;
      dt.d++;
      
      // Получаем количество дней в текущем месяце
      int daysInMonth = getDaysInMonth(dt.m, dt.y);
      
      // Проверка переполнения дней -> месяц
      if (dt.d > daysInMonth) {
        dt.d = 1;
        dt.m++;
        
        // Проверка переполнения месяцев -> год
        if (dt.m > 12) {
          dt.m = 1;
          dt.y++;
          
          // Защита от переполнения года (максимум 9999)
          if (dt.y > 9999) {
            dt.y = 9999;
            dt.m = 12;
            dt.d = 31;
            dt.h = 23;
            dt.min = 59;
          }
        }
      }
    }
  }
}

/* ==================== Get Days in Month (учитывает високосные годы) ==================== */
int TimeCircuits::getDaysInMonth(int month, int year) {
  const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (month < 1 || month > 12) return 31; // Защита от ошибок
  
  int days = daysInMonth[month - 1];
  
  // Проверка на високосный год для февраля
  if (month == 2 && isLeapYear(year)) {
    days = 29;
  }
  
  return days;
}

/* ==================== Present Time Update ==================== */
void TimeCircuits::updatePresentTime() {
  if (!presT.valid) return;

  #ifdef USE_RTC_DS3231
    if (rtc.begin()) {
      DateTime rtcNow = rtc.now();
      uint8_t currentMinute = rtcNow.minute();
      
      if (currentMinute != lastRTCMinute) {
        lastRTCMinute = currentMinute;
        
        if (useRTCForDate) {
          // ===== РЕЖИМ ПОЛНОЙ ДАТЫ: Читаем время напрямую из RTC =====
          presT.y = rtcNow.year();
          presT.m = rtcNow.month();
          presT.d = rtcNow.day();
          presT.h = rtcNow.hour();
          presT.min = rtcNow.minute();
          presT.valid = true;
        } else {
          // ===== РЕЖИМ ТАЙМЕРА: Только инкремент нашей даты =====
          incrementTime(presT);
        }
        
        refresh();
      }
      return;
    }
  #endif
  
  unsigned long ms = millis();
  
  // Каждую минуту (60 секунд)
  if (ms - tMin >= 60000UL) {
    tMin = ms;
    // Используем функцию инкремента с полной логикой даты
    incrementTime(presT);
    refresh();
    
    // Выводим в Serial для отладки (опционально, можно закомментировать)
    // Serial.print(F("⏰ Present Time updated: "));
    // Serial.println(presT.toText());
  }
}

/* ==================== Public Methods - Setters ==================== */
void TimeCircuits::setDestTime(const TCDateTime& dt) {
  destT = dt;
  refresh();
  Serial.print(F("Destination Time set: "));
  Serial.println(dt.toText());
}

void TimeCircuits::clearDestTime() {
  destT = TCDateTime();
  refresh();
  Serial.println(F("Destination Time cleared"));
}

void TimeCircuits::setPresTime(const TCDateTime& dt) {
  presT = dt;
  tMin = millis();

  #ifdef USE_RTC_DS3231
    if (rtc.begin()) {
      // Проверяем, попадает ли дата в диапазон RTC (2000-2099)
      if (dt.y >= 2000 && dt.y <= 2099) {
        // ===== Дата в диапазоне: используем RTC полностью =====
        useRTCForDate = true;
        
        // Устанавливаем точную дату в RTC
        DateTime rtcTime(dt.y, dt.m, dt.d, dt.h, dt.min, 0);
        rtc.adjust(rtcTime);
        lastRTCMinute = dt.min;
        
        Serial.println(F("RTC: Full date mode (2000-2099)"));
        Serial.print(F("Synchronized to: "));
        Serial.println(dt.toText());
      } else {
        // ===== Дата вне диапазона: используем RTC только как таймер =====
        useRTCForDate = false;
        
        // Устанавливаем фиксированную дату в RTC (для тиков)
        rtc.adjust(DateTime(2020, 1, 1, 0, 0, 0));
        DateTime rtcNow = rtc.now();
        lastRTCMinute = rtcNow.minute();
        
        Serial.println(F("RTC: Timer-only mode (year < 2000 or > 2099)"));
        Serial.print(F("Present Time: "));
        Serial.println(dt.toText());
      }
    }
  #endif

  refresh();
  Serial.print(F("Present Time set: "));
  Serial.println(dt.toText());
}

void TimeCircuits::clearPresTime() {
  presT = TCDateTime();
  refresh();
  Serial.println(F("Present Time cleared"));
}

void TimeCircuits::setLastTime(const TCDateTime& dt) {
  lastT = dt;
  refresh();
  Serial.print(F("Last Time Departed set: "));
  Serial.println(dt.toText());
}

void TimeCircuits::clearLastTime() {
  lastT = TCDateTime();
  refresh();
  Serial.println(F("Last Time Departed cleared"));
}

/* ==================== Time Travel ==================== */
void TimeCircuits::timeTravel() {
  if (!canTimeTravel()) {
    Serial.println(F("Cannot travel: invalid times or jump locked"));
    return;
  }
  
  lastT = presT;
  presT = destT;

  tMin = millis();

  #ifdef USE_RTC_DS3231
    // НЕ меняем RTC при прыжке! Только синхронизируем lastRTCMinute
    if (rtc.begin()) {
      DateTime rtcNow = rtc.now();
      lastRTCMinute = rtcNow.minute();
      //
      useRTCForDate = false; // После прыжка всегда режим таймера!
      Serial.println(F("RTC: Timer mode (after jump)"));

    }
  #endif

  jumpLock = true;
  refresh();
  
  Serial.println(F("⚡ GO TO THE FUTURE ⚡"));
  Serial.print(F("Destination Time: "));
  Serial.println(destT.toText());
  Serial.print(F("Present Time: "));
  Serial.println(presT.toText());
  Serial.print(F("Last Departed Time: "));
  Serial.println(lastT.toText());
}

/* ==================== Display Refresh ==================== */
void TimeCircuits::refresh() {
  toBufferDest(destT);
  toBufferPres(presT);
  toBufferLast(lastT);
}

/* ==================== Initialization ==================== */
void TimeCircuits::init() {
  pinMode(TC_DATA_PIN, OUTPUT);
  pinMode(TC_LATCH_PIN, OUTPUT);
  pinMode(TC_CLOCK_PIN, OUTPUT);
  
  const byte ledPins[] = {
    LED_AM_DEST, LED_PM_DEST, LED_SEC1_DEST, LED_SEC2_DEST,
    LED_AM_PRES, LED_PM_PRES, LED_SEC1_PRES, LED_SEC2_PRES,
    LED_AM_LAST, LED_PM_LAST, LED_SEC1_LAST, LED_SEC2_LAST
  };
  
  for (byte i = 0; i < sizeof(ledPins); i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  #ifdef USE_RTC_DS3231
    // Инициализация RTC модуля
    if (!rtc.begin()) {
      Serial.println(F("RTC DS3231 not found!"));
      Serial.println(F("Falling back to millis() mode"));
    } else {
      Serial.println(F("RTC DS3231 initialized"));
      
      // Проверка потери питания
      if (rtc.lostPower()) {
        Serial.println(F("RTC lost power, setting default time"));
        // Установить время компиляции как начальное (опционально)
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        rtc.adjust(DateTime(F("Oct 26 1985"), F("22:10:00")));
      }

      DateTime now = rtc.now();
      lastRTCMinute = now.minute();
    }
  #else
    Serial.println(F("Using millis() for timekeeping"));
  #endif
  
  refresh();
  Serial.println(F("Time Circuits Ready"));
}

void TimeCircuits::syncPresTimeFromRTC() {
  #ifdef USE_RTC_DS3231
    if (rtc.begin()) {
      DateTime rtcNow = rtc.now();

      // Проверяем валидность времени в RTC
      if (rtcNow.isValid() && rtcNow.year() >= 2000 && rtcNow.year() <= 2099) {
        // Устанавливаем время из RTC
        presT.y = rtcNow.year();
        presT.m = rtcNow.month();
        presT.d = rtcNow.day();
        presT.h = rtcNow.hour();
        presT.min = rtcNow.minute();
        presT.valid = true;

        useRTCForDate = true;
        lastRTCMinute = rtcNow.minute();
        tMin = millis();

        refresh();

        Serial.println(F("Present Time synced from RTC"));
        Serial.print(F("Current time: "));
        Serial.println(presT.toText());
      } else {
        Serial.println(F("RTC time invalid or out of range (2000-2099)"));
      }
    } else {
      Serial.println(F("RTC not found!"));
    }
  #else
    Serial.println(F("⚠️  RTC not enabled (USE_RTC_DS3231 not defined)"));
  #endif
}

/* ==================== Main Update Loop ==================== */
void TimeCircuits::update() {
  // Мультиплексирование дисплея
  unsigned long us = micros();
  if (us - tDig >= DIGIT_US) {
    tDig = us;
    showDigit(curDig);
    if (++curDig >= 39) curDig = 0;
  }
  
  // Мигание секундных точек
  unsigned long ms = millis();
  if (ms - tBlink >= BLINK_MS) {
    tBlink = ms;
    blinkTick = !blinkTick;
  }
  
  // Обновление LED индикаторов
  setLeds(destT, LED_AM_DEST, LED_PM_DEST, LED_SEC1_DEST, LED_SEC2_DEST, false);
  setLeds(presT, LED_AM_PRES, LED_PM_PRES, LED_SEC1_PRES, LED_SEC2_PRES, true);
  setLeds(lastT, LED_AM_LAST, LED_PM_LAST, LED_SEC1_LAST, LED_SEC2_LAST, false);
  
  // Обновление времени Present Time
  updatePresentTime();
}

// Глобальный экземпляр
TimeCircuits timeCircuits;