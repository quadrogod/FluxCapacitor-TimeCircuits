#include <Arduino.h>
#include <Keypad.h>
#include "Config.h"
#include "Globals.h"
#include "Animations.h"
#include "KeyHandler.h"
#include "IRHandler.h"
#include "TimeCircuits.h"
#include "TimeUtils.h"

// KEYPAD_AZ_DELIVERY Keypad pins (10 pin)
// NA C1 C2 C3 C4 R1 R2 R3 R4 NA

// Выберите тип клавиатуры (закомментируйте ненужный)
// #define KEYPAD_AZ_DELIVERY    // Для AZ-Delivery
#define KEYPAD_STANDARD    // Для стандартной Arduino keypad

#define DEBOUNCE_MS   200UL

unsigned long tKey=0;

const byte ROWS = 4, COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','D'},
  {'4','5','6','P'},
  {'7','8','9','L'},
  {'R','0','#','E'}
};

// sta
// byte rowPins[ROWS] = {2,3,4,5};
// byte colPins[COLS] = {6,7,8,9};
// https://cdn.shopify.com/s/files/1/1509/1638/files/4x4_Keypad_Tastatur_Datenblatt_AZ-Delivery_Vertriebs_GmbH.pdf?v=1608629136
// byte rowPins[ROWS] = {5, 4, 3, 2};    // R1=D5, R2=D4, R3=D3, R4=D2
// byte colPins[COLS] = {9, 8, 7, 6};    // C1=D9, C2=D8, C3=D7, C4=D6

// Автоматический выбор распиновки
#ifdef KEYPAD_AZ_DELIVERY
  // https://cdn.shopify.com/s/files/1/1509/1638/files/4x4_Keypad_Tastatur_Datenblatt_AZ-Delivery_Vertriebs_GmbH.pdf?v=1608629136
  // AZ-Delivery: Rows на D5-D2, Columns на D9-D6
  byte rowPins[ROWS] = {5, 4, 3, 2}; // R1=D5, R2=D4, R3=D3, R4=D2
  byte colPins[COLS] = {9, 8, 7, 6}; // C1=D9, C2=D8, C3=D7, C4=D6
#else
  // Стандартная: Rows на D2-D5, Columns на D6-D9
  byte rowPins[ROWS] = {2, 3, 4, 5};
  byte colPins[COLS] = {6, 7, 8, 9};
#endif

// or RIGHT pin connection for Standart Keypad, but physical AZ Delivery Keypad
// C1 (Pin 5 на разъеме) D6
// C2 (Pin 5 на разъеме) D7
// C3 (Pin 5 на разъеме) D8
// C4 (Pin 5 на разъеме) D9

// R1 (Pin 1 на разъеме) D2
// R2 (Pin 1 на разъеме) D3
// R3 (Pin 1 на разъеме) D4
// R4 (Pin 1 на разъеме) D5

Keypad kpd(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Режимы ввода
enum InputMode { MODE_NONE, MODE_SET_DEST, MODE_SET_PRES, MODE_SET_LAST };
InputMode inputMode = MODE_NONE;
String inDigits = "";
char lastKeyPressed = '\0';

void initKeypad() {
  Serial.println(F("Keypad Ready"));
}

void handleDateInput(char key) {
  if (key >= '0' && key <= '9' && inDigits.length() < 12) {
    inDigits += key;
    Serial.print(key);
    return;
  }
  
  if (key == '#') {
    inDigits = "";
    Serial.println(F("\nInput cancelled"));
    inputMode = MODE_NONE;
    return;
  }
  
  if (key == 'R') {
    inDigits = "";
    Serial.println(F("\nReset"));
    return;
  }
    #ifdef SET_SIMPLE_DATES
    if (key == 'D') {
        if (inputMode == MODE_SET_DEST && inDigits.length() == 0) {
            timeCircuits.setDestTime(parseDateTime("110519550600")); // синхронизируем Destination Time
            Serial.println();
            Serial.println(F("Destination Time Set as Default."));
            inputMode = MODE_NONE;
            return;
        }
        return;
    }
    #endif

  if (key == 'P') {
      if (inputMode == MODE_SET_PRES && inDigits.length() == 0) {
          timeCircuits.syncPresTimeFromRTC(); // синхронизируем Present Time
          Serial.println();
          Serial.println(F("Present Time Synchronized."));
          inputMode = MODE_NONE;
          return;
      }
    return;
  }

    #ifdef SET_SIMPLE_DATES
        if (key == 'L') {
            if (inputMode == MODE_SET_LAST && inDigits.length() == 0) {
                timeCircuits.setLastTime(parseDateTime("102619850122")); // синхронизируем Last Departed Time
                Serial.println();
                Serial.println(F("Last Departed Time Set as Default."));
                inputMode = MODE_NONE;
                return;
            }
            return;
        }
    #endif
  
  if (key == 'E') {
    if (inDigits.length() != 12) {
      Serial.println(F("\nError: Need 12 digits (MMDDYYYYHHmm)"));
      inputMode = MODE_NONE;
      inDigits = "";
      return;
    }
    
    TCDateTime dt = parseDateTime(inDigits);
    if (!dt.valid) {
      Serial.println(F("\nError: Invalid date"));
      inputMode = MODE_NONE;
      inDigits = "";
      return;
    }

    Serial.println();
    
    // Использование методов класса TimeCircuits
    if (inputMode == MODE_SET_DEST) {
      timeCircuits.setDestTime(dt);
    } else if (inputMode == MODE_SET_PRES) {
      timeCircuits.setPresTime(dt);
    } else {
      timeCircuits.setLastTime(dt);
    }

    inputMode = MODE_NONE;
    inDigits = "";
    return;
  }
}

void handleNormalMode(char k) {

    switch (k)
    {
        case '1':
            resetModes();
            setTimeTravel();
            Serial.println(F("Time Travel activated"));
            break;
            
        case '2':
            resetModes();
            setSmoothChase();
            Serial.println(F("Smooth Chase activated"));
            break;
            
        case '3':
            resetModes();
            setThirtyChase();
            Serial.println(F("Thirty Chase activated"));
            break;

        case '4':
            resetModes();
            setMovieChase();
            Serial.println(F("Movie Chase activated"));
            break;
          
        case '5':
            resetModes();
            setMovieChaseSimple();
            Serial.println(F("Movie Chase Simple activated"));
            break;

        case '6':
            resetModes();
            setRadChase();
            Serial.println(F("Rad Chase activated"));
            break;

        case '7':
            resetModes();
            setRadChase2();
            Serial.println(F("Rad Chase 2 activated"));
            break;

        case '8':
            resetModes();
            setRainbowChase();
            Serial.println(F("Rainbow activated"));
            break;

        case '9':
            resetModes();
            setMovieTimeTravel();
            Serial.println(F("Movie Time Travel activated"));
            break;

        case '0':
            resetModes();
            setOff();
            Serial.println(F("LEDs cleared"));
            break;

        case 'D':
            inputMode = MODE_SET_DEST;
            inDigits = "";
            Serial.print(F("Enter Destination Time (MMDDYYYYHHmm): "));
            break;
        
        case 'P':
            inputMode = MODE_SET_PRES;
            inDigits = "";
            Serial.print(F("Enter Present Time (MMDDYYYYHHmm): "));
            break;

        case 'L':
            inputMode = MODE_SET_LAST;
            inDigits = "";
            Serial.print(F("Enter Last Time Departed (MMDDYYYYHHmm): "));
            break;
            
        default:
            Serial.println(F("Unknown command"));
            Serial.println(k);
            break;
    }
}

void handleKey() {

    unsigned long ms = millis();

    char k = kpd.getKey();
    if (!k) return;

    if (ms - tKey < DEBOUNCE_MS && k == lastKeyPressed) return;

    tKey = ms;
    lastKeyPressed = k;

    if (inputMode == MODE_NONE) {
        handleNormalMode(k);
    } else {
        handleDateInput(k);
    }

}