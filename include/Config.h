#pragma once

/********************************************************************
 *  CONFIGURATION
 ********************************************************************/

// ----------------- DFPlayer -----------------
// SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
// DFRobotDFPlayerMini myDFPlayer;

// ==================== RTC Configuration ====================
#define USE_RTC_DS3231  // Закомментируйте эту строку, чтобы вернуться к millis() для отсчета Present Time
// условие не обязательное, т.к. я всё обернул в проверку, и если RTC не подключен, то просто будет игнорировать код

// Режим работы датчика
#define USE_STUB_SENSOR true // true = заглушка, false = реальный датчик

#define SET_SIMPLE_DATES // если переменная есть, то при установке даты по второму нажатию на клавишу инициализации ввода подставятся даты из фильмов

// ----------------- IR -----------------
#define IR_RECEIVE_PIN 10

// Remote button codes
#define BTN_0       104
#define BTN_1       48
#define BTN_2       24
#define BTN_3       122
#define BTN_4       16
#define BTN_5       56
#define BTN_6       90
#define BTN_7       66
#define BTN_8       74
#define BTN_9       82
#define BTN_STOP    168
#define BTN_VOL_UP  2
#define BTN_VOL_DWN 152
#define BTN_UP      224
#define BTN_DOWN    144
#define BTN_POWER   162

// ----------------- LEDS -----------------
// #define NUM_LEDS   22
// #define NUM_LEDS   22
// #define NUM_LEDS_ON_LINE   10
#define NUM_LEDS 11                 // Физическое количество LED в ленте
#define SKIP_LEDS 1                 // Количество пропускаемых LED с начала ленты
#define NUM_LEDS_ACTIVE (NUM_LEDS - SKIP_LEDS)  // Активных LED: 10 - 1 = 9

#define NUM_LEDS_ON_LINE 10 // @todo: надо удалить эту переменную в будущем

#define DATA_RING_PIN   11
// #define CLOCK_PIN  13
#define LED_POWER_LIMIT_MA 500

// ----------------- Misc -----------------
#define SINGLE_LED_PIN 12

// ----------------- Keypad -----------------

/* --- Пины 74HC595 ----------------------------------------- */
#define TC_DATA_PIN      22
#define TC_LATCH_PIN     23
#define TC_CLOCK_PIN     24

/* --- LED индикаторы --------------------------------------- */
#define LED_AM_DEST   30
#define LED_PM_DEST   31
#define LED_SEC1_DEST 32
#define LED_SEC2_DEST 33

#define LED_AM_PRES   34
#define LED_PM_PRES   35
#define LED_SEC1_PRES 36
#define LED_SEC2_PRES 37

#define LED_AM_LAST   38
#define LED_PM_LAST   39
#define LED_SEC1_LAST 40
#define LED_SEC2_LAST 41

/* --- Мультиплекс ------------------------------------------ */
#define SR_CNT        8        // sr1..sr8
#define DIGITS_TOTAL  39       // ИЗМЕНЕНО: было 42, стало 39 (3 индикатора по -1 разряду)
#define DIGIT_US      1500UL   // микросекунды на один разряд
#define BLINK_MS      1000UL
#define DEBOUNCE_MS   200UL

/* --- Датчик ----------------------------------------------- */
#define NTC_PIN       A0
// ==================== Temperature Speed Sensor ====================
#define TEMP_START_THRESHOLD 30.0   // Температура начала ускорения (°C)
#define TEMP_FLASH_THRESHOLD 80.0   // Температура синей вспышки (°C)
#define TEMP_RESET_THRESHOLD 20.0   // Температура сброса блокировки (°C)