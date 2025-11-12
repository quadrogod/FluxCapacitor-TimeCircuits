#include <Arduino.h>
#include <FastLED.h>
// #include <IRremote.hpp>
// #include <SoftwareSerial.h>
// #include <DFRobotDFPlayerMini.h>

#include "Config.h"
#include "Globals.h"
#include "IRHandler.h"
#include "KeyHandler.h"
#include "TemperatureHandler.h"
#include "SerialHandler.h"
#include "Animations.h"
#include "TimeCircuits.h"

void setup() {
    pinMode(SINGLE_LED_PIN, OUTPUT);

    Serial.begin(115200);
    Serial.println(F("\n=== Flux Capacitor & Time Circuits ==="));
    Serial.println(F("\nVersion: 1.24"));

    Serial.println(F("\n=== Setup Starting ==="));

    // ---------------- DFPlayer (commented for Wokwi) ----------------
    // mySoftwareSerial.begin(9600);
    // if (!myDFPlayer.begin(mySoftwareSerial)) {
    //   Serial.println(F("Unable to begin DFPlayer Mini!"));
    //   while (true);
    // }
    // myDFPlayer.volume(29);
    // myDFPlayer.play(2); // startup sound
    // delay(600);

    // ---------------- IR Receiver ----------------
    initIR();
    Serial.println(F("IR Receiver Ready."));

    // ---------------- LED Setup ----------------
    FastLED.addLeds<WS2812B, DATA_RING_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_POWER_LIMIT_MA);
    FastLED.clear();
    FastLED.show();

    timeCircuits.init();
    initKeypad();
    initTemperatureSensor();

    Serial.println(F("Setup Completed.\n"));

    // запускаем свет
    resetModes();   
    setMovieChaseSimple();
    Serial.println(F("Movie Chase Simple activated"));    
}

void loop() {
    handleIRRemote();
    handleKey();
    handleSerial();
    handleAnimations();
    handleTemperatureSpeed();
    timeCircuits.update(); 
}