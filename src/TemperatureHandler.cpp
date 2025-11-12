#include <Arduino.h>
#include <FastLED.h>
#include "Config.h"
#include "TimeCircuits.h"
#include "Animations.h"
#include "TemperatureHandler.h"

const float BETA_THERM = 3950;

// Настройки температуры (можно менять динамически)
float tempStartThreshold = TEMP_START_THRESHOLD;
float tempFlashThreshold = TEMP_FLASH_THRESHOLD;
float tempResetThreshold = TEMP_RESET_THRESHOLD;

// Текущее состояние
float currentTemp = 0;
bool speedAnimationActive = false;

float getTemperature() {
  int raw = analogRead(NTC_PIN);
  float tempC = 1 / (log(1 / (1023. / raw - 1)) / BETA_THERM + 1.0 / 298.15) - 273.15;
  return tempC;
}

void initTemperatureSensor() {
  pinMode(NTC_PIN, INPUT);
  Serial.println(F("🌡️  Temperature Speed Sensor Ready"));
  Serial.print(F("   Start threshold: "));
  Serial.print(tempStartThreshold);
  Serial.println(F("°C"));
  Serial.print(F("   Flash threshold: "));
  Serial.print(tempFlashThreshold);
  Serial.println(F("°C"));
}

// Прогресс от START до FLASH (0.0 - 1.0)
float getTempProgress() {
  if (currentTemp <= tempStartThreshold) return 0.0;
  if (currentTemp >= tempFlashThreshold) return 1.0;
  
  float range = tempFlashThreshold - tempStartThreshold;
  float progress = (currentTemp - tempStartThreshold) / range;
  return constrain(progress, 0.0, 1.0);
}

void handleTemperatureSpeed() {
  currentTemp = getTemperature();
  
  // ===== ЗАПУСК АНИМАЦИИ =====
  if (!speedAnimationActive && currentTemp >= tempStartThreshold && timeCircuits.canTimeTravel()) {
    speedAnimationActive = true;
    setMovieTimeTravelSpeed(); // Новая функция анимации!
    
    Serial.println(F("🔥 Temperature Speed Mode Activated!"));
    Serial.print(F("   Current temp: "));
    Serial.print(currentTemp);
    Serial.println(F("°C"));
  }
  
  // ===== СБРОС БЛОКИРОВКИ =====
  if (currentTemp < tempResetThreshold && timeCircuits.isJumpLocked()) {
    timeCircuits.unlockJump();
    speedAnimationActive = false;
    
    Serial.println(F("❄️  Temperature dropped - Jump unlocked"));
  }
}
