#include <Arduino.h>
#include <FastLED.h>
#include "Config.h"
#include "Globals.h"
#include "core/Logger/SerialLogger.h"
#include "core/Sensor/TemperatureSensor.h"
#include "core/RTC/DS3231RTCProvider.h"
#include "IRHandler.h"
#include "KeyHandler.h"
#include "SerialHandler.h"
#include "Animations.h"
#include "TimeCircuits.h"
// #include <IRremote.hpp>
// #include <SoftwareSerial.h>
// #include <DFRobotDFPlayerMini.h>
#include "TemperatureHandler.h"

// Логгер
SerialLogger logger;
// Сенсор
TemperatureSensor tempSensor;
// RTC
DS3231RTCProvider rtcProvider;

void setup() {
    pinMode(SINGLE_LED_PIN, OUTPUT);
    Serial.begin(115200);

    logger.println(F("=== Flux Capacitor & Time Circuits ==="));
    logger.println(F("Version: 1.25"));

    logger.println(F("=== Setup Starting ==="));

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
    logger.println(F("IR Receiver Ready."));

    // ---------------- LED Setup ----------------
    FastLED.addLeds<WS2812B, DATA_RING_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_POWER_LIMIT_MA);
    FastLED.clear();
    FastLED.show();

    timeCircuits.init();
    initKeypad();
    initTemperatureSensor();

    logger.println(F("Setup Completed."));

    // запускаем свет
    resetModes();   
    setMovieChaseSimple();
    logger.println(F("Movie Chase Simple activated"));
}

void loop() {
    handleIRRemote();
    handleKey();
    handleSerial();
    handleAnimations();
    handleTemperatureSpeed();
    timeCircuits.update(); 
}