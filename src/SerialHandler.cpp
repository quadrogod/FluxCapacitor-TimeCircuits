#include <FastLED.h>
#include "Config.h"
#include "Animations.h"
// #include "Globals.h"
// #include "IRHandler.h"

void handleSerial() {
    if (!Serial.available()) return;
    
    char k = Serial.read();
    
    // Очистка буфера от лишних символов (например \n)
    while (Serial.available()) Serial.read();
    
    Serial.print(F("Received: "));
    Serial.println(k);
    
    switch (k) {
        case '1':
            setTimeTravel();
            Serial.println(F("Time Travel activated"));
            break;
            
        case '2':
            setSmoothChase();
            Serial.println(F("Smooth Chase activated"));
            break;
            
        case '3':
            setThirtyChase();
            Serial.println(F("Thirty Chase activated"));
            break;

        case '4':
            setMovieChase();
            Serial.println(F("Movie Chase activated"));
            break;
          
        case '5':
            setMovieChaseSimple();
            Serial.println(F("Movie Chase Simple activated"));
            break;

        case '6':
            setRadChase();
            Serial.println(F("Rad Chase activated"));
            break;

        case '7':
            setRadChase2();
            Serial.println(F("Rad Chase 2 activated"));
            break;

        case '8':
            setRainbowChase();
            Serial.println(F("Rainbow activated"));
            break;

        case '9':
            setMovieTimeTravel();
            Serial.println(F("Movie Time Travel activated"));
            break;

        case '0':
            setOff();
            Serial.println(F("LEDs cleared"));
            break;
            
        default:
            Serial.println(F("Unknown command"));
            Serial.println(k);
            break;
    }
}
