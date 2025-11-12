#ifndef TEMPERATUREHANDLER_H
#define TEMPERATUREHANDLER_H

// Пороги температуры
extern float tempStartThreshold;
extern float tempFlashThreshold;
extern float tempResetThreshold;

// Текущие данные
extern float currentTemp;
extern bool speedAnimationActive;

void initTemperatureSensor();
float getTemperature();
void handleTemperatureSpeed();
float getTempProgress(); // Прогресс от START до FLASH (0.0 - 1.0)

#endif