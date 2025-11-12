#include <Arduino.h>
#include <FastLED.h>
#include <IRremote.hpp>
#include "Config.h"
#include "Globals.h"
#include "Animations.h"
#include "IRHandler.h"

void initIR() {
    IrReceiver.begin(IR_RECEIVE_PIN);
}

void handleIRRemote() {
    if (!IrReceiver.decode()) return;

    uint8_t cmd = IrReceiver.decodedIRData.command;
    Serial.print(F("IR Command: "));
    Serial.println(cmd);

    // myDFPlayer.stop(); // generic stop before switching mode

    resetModes();

    switch (cmd) {
        case BTN_0:
            setOff();
            Serial.println(F("LEDs cleared"));
            break;

        case BTN_1:  
            setTimeTravel();
            Serial.println(F("Time Travel activated"));
            break;

        case BTN_POWER: FastLED.clear(); FastLED.show(); break;

        case BTN_2:  
            setSmoothChase();
            Serial.println(F("Smooth Chase activated")); 
            break;

        case BTN_3:  
            setThirtyChase();
            Serial.println(F("Thirty Chase activated"));
            break;

        case BTN_4: 
            setMovieChase();
            Serial.println(F("Movie Chase activated"));
            break;

        case BTN_5:  
            setMovieChaseSimple();
            Serial.println(F("Movie Chase Simple activated"));
            break;

        case BTN_6:  
            setRadChase();
            Serial.println(F("Rad Chase activated"));
            break;

        case BTN_7:  
            setRadChase2();
            Serial.println(F("Rad Chase 2 activated"));
            break;

        case BTN_8:  
            setRainbowChase();
            Serial.println(F("Rainbow activated"));
            break;

        case BTN_9:
            // myDFPlayer.playMp3Folder(6);
            setMovieTimeTravel();
            Serial.println(F("Movie Time Travel activated"));
            break;

        // case BTN_UP:
        //     if (movieSpeed > 24) movieSpeed -= 10;
        //     if (delaySpeed > 20) delaySpeed -= (delaySpeed <= 20 ? 4 : 20);
        //     Serial.println(F("↑ Speed up"));
        //     break;

        // case BTN_DOWN:
        //     if (movieSpeed < 200) movieSpeed += 10;
        //     if (delaySpeed < 160) delaySpeed += 20;
        //     Serial.println(F("↓ Slow down"));
        //     break;

        default: break;
    }

    IrReceiver.resume();
}
