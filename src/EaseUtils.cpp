//
// Created by Alexey Molchanov on 13.11.2025.
//

#include <Arduino.h>
#include "EaseUtils.h"

// Ease-In Cubic: медленный старт, резкое ускорение в конце
float easeInCubic(float t) {
    return t * t * t;
}

// Ease-In-Out Cubic: S-образная кривая (опционально)
float easeInOutCubic(float t) {
    return t < 0.5
      ? 4 * t * t * t
      : 1 - pow(-2 * t + 2, 3) / 2;
}

// Ease-In Exponential: очень медленный старт, ОЧЕНЬ быстрый конец
float easeInExpo(float t) {
    return t == 0.0 ? 0.0 : pow(2, 10 * (t - 1));
}

// Custom: ваша специфичная кривая
float customSpeedCurve(float t) {
    // До 0.6 (60% диапазона = ~60°C): очень медленный рост
    // После 0.6: резкое ускорение
    if (t < 0.6) {
        // Медленный рост: квадратичная функция
        return 0.15 * (t / 0.6) * (t / 0.6); // 0.0 → 0.15
    } else {
        // Быстрый рост: кубическая функция
        float normalizedT = (t - 0.6) / 0.4; // 0.0 → 1.0
        return 0.15 + 0.85 * normalizedT * normalizedT * normalizedT; // 0.15 → 1.0
    }
}