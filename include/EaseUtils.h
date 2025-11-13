//
// Created by Alexey Molchanov on 13.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_EASEUTILS_H
#define FLUXCAPACITOR_TIMECIRCUITS_EASEUTILS_H

#include <Arduino.h>

// Ease-In Cubic: медленный старт, резкое ускорение в конце
float easeInCubic(float t);

// Ease-In-Out Cubic: S-образная кривая (опционально)
float easeInOutCubic(float t);

// Ease-In Exponential: очень медленный старт, ОЧЕНЬ быстрый конец
float easeInExpo(float t);

// Custom: ваша специфичная кривая
float customSpeedCurve(float t);

#endif //FLUXCAPACITOR_TIMECIRCUITS_EASEUTILS_H