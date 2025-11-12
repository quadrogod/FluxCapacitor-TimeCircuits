#include <FastLED.h>
#include <GTimer.h>
#include "Config.h"
#include "Globals.h"
#include "Animations.h"
#include "TemperatureHandler.h"
#include "TimeCircuits.h"

// ==================== Easing Functions ====================
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

// Состояния для Movie Time Travel эффекта
enum class TTState : uint8_t {
    RUNNING,        // Основная анимация ускорения
    FLASH_START,    // Начало яркой вспышки (88 mph)
    FLASH_HOLD,     // Удержание яркой вспышки
    FLASH_FADE,     // Плавное затухание перед взрывами (ИЗМЕНЕНО)
    DARK_1,         // Темнота перед первым взрывом
    BURST_1,        // Первый взрыв (короткая вспышка)
    DARK_2,         // Темнота между взрывами
    BURST_2,        // Второй взрыв (ярче и дольше)
    DARK_3,         // Темнота перед появлением
    BURST_3,        // Третий взрыв - появление (самый яркий)
    FADE_OUT,       // Плавное затухание
    COMPLETE        // Завершение, переход на следующий режим
};

static TTState ttState = TTState::RUNNING;
static GTimer<millis> ttTimer;
// static uint8_t fadeBrightness = 255;
static unsigned long fadeStartTime = 0; // Для отслеживания времени фейда
//

CRGB leds[NUM_LEDS];
uint8_t hue = 0;

// Глобальные переменные для анимаций
int delaySpeed = 80;
float movieSpeed = 34.45;

// Флаги режимов
bool timeTravel = false;
bool movieTimeTravel = false;
bool smoothChase = false;
bool movieChase = false;
bool movieChaseSimple = false;
bool thirtyChase = false;
bool radChase = false;
bool radChase2 = false;
bool rainbowChase = false;
bool movieTimeTravelSpeed = false;

// Таймеры для анимаций
GTimer<millis> animTimer;
GTimer<millis> hueTimer(5, true); // для rainbow

// Счетчики кадров анимаций
int animStep = 0;
int animSubStep = 0;

// Для версии с искрами (Real2)
static int sparkCounter = 0;           // Счётчик для отслеживания фазы искр
static unsigned long lastSparkTime = 0; // Время последней искры
static int sparkPixel = -1;            // Текущий пиксель искры
static uint8_t sparkBrightness = 0;    // Яркость искры для затухания

void resetModes() {
    timeTravel = smoothChase = movieChase = movieChaseSimple =
    thirtyChase = radChase = radChase2 = rainbowChase = movieTimeTravel = movieTimeTravelSpeed = false;
    animStep = 0;
    animSubStep = 0;
    animTimer.stop();
    // Сброс Time Travel состояния
    ttState = TTState::RUNNING;
    ttTimer.stop();
}

void setSmoothChase() {
    resetModes();
    smoothChase = true;
    delaySpeed = 80;
    animTimer.setTime(delaySpeed);
    animTimer.start();
}

void setMovieChase() {
    resetModes();
    movieChase = true;
    movieSpeed = 22.97;
    animTimer.setTime(movieSpeed);
    animTimer.start();
}

void setMovieChaseSimple() {
    resetModes();
    movieChaseSimple = true;
    movieSpeed = 34.45;
    animTimer.setTime(movieSpeed);
    animTimer.start();
}

void setThirtyChase() {
    resetModes();
    thirtyChase = true;
    movieSpeed = 33.33;
    animTimer.setTime(movieSpeed);
    animTimer.start();
}

void setRadChase() {
    resetModes();
    radChase = true;
    movieSpeed = 66.66;
    animTimer.setTime(movieSpeed);
    animTimer.start();
}

void setRadChase2() {
    resetModes();
    radChase2 = true;
    movieSpeed = 66.66;
    animTimer.setTime(movieSpeed);
    animTimer.start();
}

void setTimeTravel() {
    resetModes();
    timeTravel = true;
    delaySpeed = 113;
    animTimer.setTime(delaySpeed);
    animTimer.start();
}

void setMovieTimeTravel() {
    resetModes();
    movieTimeTravel = true;
    delaySpeed = 113;
    animTimer.setTime(delaySpeed);
    animTimer.start();
}

void setMovieTimeTravelSpeed() {
    resetModes();
    movieTimeTravelSpeed = true;
    delaySpeed = 113;
    animTimer.setTime(delaySpeed);
    animTimer.start();
}

void setRainbowChase() {
    resetModes();
    rainbowChase = true;
    movieSpeed = 66.66;
}

void setOff() {
    resetModes();
    FastLED.clear();
    FastLED.show();
}

void handleAnimations() {
    if (smoothChase) runSmoothChase();
    else if (movieChase) runMovieChase();
    else if (movieChaseSimple) runMovieChaseSimple();
    else if (thirtyChase) runThirtyChase();
    else if (radChase) runRadChase();
    else if (radChase2) runRadChase2();
    else if (timeTravel) runTimeTravel();
    // else if (movieTimeTravel) runMovieTimeTravel();
    // else if (movieTimeTravel) runMovieTimeTravelReal();
    else if (movieTimeTravel) runMovieTimeTravelReal2();
    else if (movieTimeTravelSpeed) runMovieTimeTravelSpeed();
    else if (rainbowChase) runRainbowChase();
}

void runSmoothChase() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0) {
            leds[animStep - j] = CHSV(32, 128, 20 + j * 30);
        }
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= NUM_LEDS) animStep = 0;
}

void runMovieChase() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    int idx = animStep * 2;
    if (idx < NUM_LEDS - 1) {
        leds[idx] = leds[idx + 1] = CHSV(22, 200, 100);
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= 6) animStep = 0;
}

void runMovieChaseSimple() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    int idx = 2 + animStep * 2;
    if (idx <= 8) {
        leds[idx] = CHSV(22, 200, 100);
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= 4) animStep = 0;
}

void runThirtyChase() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    int base = animStep * 2;
    if (base < NUM_LEDS - 1) {
        leds[base] = CHSV(32, 128, 100);
        leds[base + 1] = CHSV(32, 128, 100);
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= 4) animStep = 0;
}

void runRadChase() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    int base = animStep * 2;
    if (base < NUM_LEDS - 1) {
        leds[base] = leds[base + 1] = CHSV(28, 120, 100);
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= 5) animStep = 0;
}

void runRadChase2() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    if (animStep < NUM_LEDS) {
        leds[animStep] = CHSV(28, 200, 120);
        if (animStep > 0) leds[animStep - 1] = CHSV(28, 200, 30);
        if (animStep < NUM_LEDS - 1) leds[animStep + 1] = CHSV(28, 200, 30);
    }
    FastLED.show();
    
    animStep++;
    if (animStep >= 10) animStep = 0;
}

void runRainbowChase() {
    // Обновление hue каждые 5 мс
    if (hueTimer.tick()) hue++;
    
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + i * 10, 255, 200);
    }
    FastLED.show();
}

void runTimeTravel() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
            leds[animStep - j] = CHSV(28, 150, 60 + j * 30);
        }
    }
    FastLED.show();
    
    animStep++;
    
    if (animStep >= NUM_LEDS) {
        delaySpeed *= 0.837;
        animTimer.setTime(delaySpeed);
        animStep = 0;
        
        if (delaySpeed < 1) {
            // Финальная вспышка
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.show();
            delay(3300); // Единственный delay для финальной вспышки
            FastLED.clear();
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, LOW);
            
            // Переход на следующий режим
            delaySpeed = 80;
            setSmoothChase();
        }
    }
}

void runMovieTimeTravel() {
    if (!animTimer.tick()) return;
    
    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
            leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
        }
    }
    FastLED.show();
    
    animStep++;
    
    if (animStep >= NUM_LEDS) {
        delaySpeed *= 0.837;
        animTimer.setTime(delaySpeed);
        animStep = 0;
        
        if (delaySpeed < 1) {
            // Финальная вспышка
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.show();
            delay(3300); // Единственный delay для финальной вспышки
            FastLED.clear();
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, LOW);
            // first Brust
            delay(400);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, HIGH);
            delay(1000);
            FastLED.clear();
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, LOW);
            // second Brust
            delay(400);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, HIGH);
            delay(1000);
            FastLED.clear();
            FastLED.show();
            digitalWrite(SINGLE_LED_PIN, LOW);
            delay(400);
            // Переход на следующий режим
            delaySpeed = 80;
            movieSpeed = 66.66;
            setRadChase2();
        }
    }
}

// void runMovieTimeTravelReal() {
//     // Основная анимация Time Travel (ускорение)
//     if (ttState == TTState::RUNNING) {
//         if (!animTimer.tick()) return;
        
//         FastLED.clear();
//         for (int j = 0; j <= 6; j++) {
//             if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
//                 leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
//             }
//         }
//         FastLED.show();
        
//         animStep++;
        
//         if (animStep >= NUM_LEDS) {
//             delaySpeed *= 0.837;
//             animTimer.setTime(delaySpeed);
//             animStep = 0;
            
//             // Когда скорость достигает максимума - начинаем финальную последовательность
//             if (delaySpeed < 1) {
//                 ttState = TTState::FLASH_START;
//                 ttTimer.start(50); // Небольшая задержка перед вспышкой
//                 animTimer.stop();
//                 return;
//             }
//         }
//         return;
//     }
    
//     // ========================================================================
//     // ФИНАЛЬНАЯ ПОСЛЕДОВАТЕЛЬНОСТЬ ВСПЫШЕК (как в фильме)
//     // ========================================================================
    
//     if (!ttTimer.tick()) return;
    
//     switch (ttState) {
//         case TTState::FLASH_START:
//             // Момент достижения 88 mph - яркая синяя вспышка
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::Blue);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FLASH_HOLD;
//             ttTimer.start(1800); // Долгое яркое свечение
//             break;
            
//         case TTState::FLASH_HOLD:
//             // Удерживаем яркую вспышку
//             ttState = TTState::FLASH_FADE;
//             ttTimer.start(500); // Начало затухания
//             break;
            
//         case TTState::FLASH_FADE:
//             // Легкое затухание перед взрывами
//             FastLED.setBrightness(180);
//             FastLED.show();
//             ttState = TTState::DARK_1;
//             ttTimer.start(200); // Короткая пауза
//             break;
            
//         case TTState::DARK_1:
//             // Темнота перед первым взрывом
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_1;
//             ttTimer.start(150); // Короткая темнота
//             break;
            
//         case TTState::BURST_1:
//             // Первый взрыв - короткая яркая вспышка (бело-голубая)
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_2;
//             ttTimer.start(180); // Длительность первого взрыва
//             break;
            
//         case TTState::DARK_2:
//             // Темнота между первым и вторым взрывом
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_2;
//             ttTimer.start(250); // Пауза между взрывами
//             break;
            
//         case TTState::BURST_2:
//             // Второй взрыв - ярче и дольше
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_3;
//             ttTimer.start(220); // Длительность второго взрыва
//             break;
            
//         case TTState::DARK_3:
//             // Темнота перед появлением DeLorean
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_3;
//             ttTimer.start(180); // Короткая пауза перед появлением
//             break;
            
//         case TTState::BURST_3:
//             // Третий взрыв - появление DeLorean (самый яркий и длинный)
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::White); // Чисто белый
//             FastLED.setBrightness(255);
//             FastLED.show();
//             fadeBrightness = 255;
//             ttState = TTState::FADE_OUT;
//             ttTimer.start(450); // Длинная вспышка появления
//             break;
            
//         case TTState::FADE_OUT:
//             // Плавное затухание после появления
//             if (fadeBrightness > 0) {
//                 fadeBrightness -= 15;
//                 if (fadeBrightness < 15) fadeBrightness = 0;
//                 FastLED.setBrightness(fadeBrightness);
//                 FastLED.show();
//                 ttTimer.start(25); // Шаг затухания
//             } else {
//                 digitalWrite(SINGLE_LED_PIN, LOW);
//                 FastLED.clear();
//                 FastLED.show();
//                 ttState = TTState::COMPLETE;
//                 ttTimer.start(400); // Финальная пауза
//             }
//             break;
            
//         case TTState::COMPLETE:
//             // Восстановление и переход на следующий режим
//             FastLED.setBrightness(255);
//             delaySpeed = 80;
//             movieSpeed = 66.66;
//             ttState = TTState::RUNNING; // Сброс состояния
//             setRadChase2(); // Переход на RadChase2
//             break;
            
//         default:
//             break;
//     }
// }

// void runMovieTimeTravelReal() {
//     // Основная анимация Time Travel (ускорение)
//     if (ttState == TTState::RUNNING) {
//         if (!animTimer.tick()) return;
        
//         FastLED.clear();
//         for (int j = 0; j <= 6; j++) {
//             if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
//                 leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
//             }
//         }
//         FastLED.show();
        
//         animStep++;
        
//         if (animStep >= NUM_LEDS) {
//             delaySpeed *= 0.837;
//             animTimer.setTime(delaySpeed);
//             animStep = 0;
            
//             if (delaySpeed < 1) {
//                 ttState = TTState::FLASH_START;
//                 ttTimer.start(50);
//                 animTimer.stop();
//                 return;
//             }
//         }
//         return;
//     }
    
//     // ========================================================================
//     // СПЕЦИАЛЬНАЯ ОБРАБОТКА ПЛАВНОГО ЗАТУХАНИЯ
//     // ========================================================================
    
//     if (ttState == TTState::FLASH_FADE) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 500; // 500 мс для плавного затухания
        
//         if (elapsed < fadeDuration) {
//             // Плавная интерполяция от 255 до 180 за 500 мс
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return; // Продолжаем затухание
//         } else {
//             // Затухание завершено
//             ttState = TTState::DARK_1;
//             ttTimer.start(200);
//             return;
//         }
//     }
    
//     // ========================================================================
//     // ОБЫЧНАЯ ОБРАБОТКА ОСТАЛЬНЫХ СОСТОЯНИЙ
//     // ========================================================================
    
//     if (!ttTimer.tick()) return;
    
//     switch (ttState) {
//         case TTState::FLASH_START:
//             // Момент достижения 88 mph - яркая синяя вспышка
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::Blue);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FLASH_HOLD;
//             ttTimer.start(1800); // Долгое яркое свечение
//             break;
            
//         case TTState::FLASH_HOLD:
//             // Удерживаем яркую вспышку
//             ttState = TTState::FLASH_FADE;
//             fadeStartTime = millis(); // Запоминаем время начала фейда
//             // НЕ используем ttTimer здесь - плавное затухание обрабатывается выше
//             break;
            
//         case TTState::DARK_1:
//             // Темнота перед первым взрывом
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_1;
//             ttTimer.start(150);
//             break;
            
//         case TTState::BURST_1:
//             // Первый взрыв - короткая яркая вспышка (бело-голубая)
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_2;
//             ttTimer.start(180);
//             break;
            
//         case TTState::DARK_2:
//             // Темнота между первым и вторым взрывом
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_2;
//             ttTimer.start(250);
//             break;
            
//         case TTState::BURST_2:
//             // Второй взрыв - ярче и дольше
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_3;
//             ttTimer.start(220);
//             break;
            
//         case TTState::DARK_3:
//             // Темнота перед появлением DeLorean
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_3;
//             ttTimer.start(180);
//             break;
            
//         case TTState::BURST_3:
//             // Третий взрыв - появление DeLorean (самый яркий и длинный)
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::White);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FADE_OUT;
//             fadeStartTime = millis(); // Запоминаем время начала финального фейда
//             ttTimer.start(450);
//             break;
            
//         case TTState::FADE_OUT:
//             // Плавное затухание после появления
//             {
//                 unsigned long elapsed = millis() - fadeStartTime;
//                 const unsigned long fadeDuration = 600; // 600 мс для плавного затухания
                
//                 if (elapsed < fadeDuration) {
//                     // Плавная интерполяция от 255 до 0
//                     uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
//                     FastLED.setBrightness(brightness);
//                     FastLED.show();
//                 } else {
//                     digitalWrite(SINGLE_LED_PIN, LOW);
//                     FastLED.clear();
//                     FastLED.show();
//                     ttState = TTState::COMPLETE;
//                     ttTimer.start(400);
//                 }
//             }
//             break;
            
//         case TTState::COMPLETE:
//             // Восстановление и переход на следующий режим
//             FastLED.setBrightness(255);
//             delaySpeed = 80;
//             movieSpeed = 66.66;
//             ttState = TTState::RUNNING;
//             setRadChase2();
//             break;
            
//         default:
//             break;
//     }
// }

// void runMovieTimeTravelReal() {
//     // Основная анимация Time Travel (ускорение)
//     if (ttState == TTState::RUNNING) {
//         if (!animTimer.tick()) return;
        
//         FastLED.clear();
//         for (int j = 0; j <= 6; j++) {
//             if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
//                 leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
//             }
//         }
//         FastLED.show();
        
//         animStep++;
        
//         if (animStep >= NUM_LEDS) {
//             delaySpeed *= 0.837;
//             animTimer.setTime(delaySpeed);
//             animStep = 0;
            
//             if (delaySpeed < 1) {
//                 ttState = TTState::FLASH_START;
//                 ttTimer.start(50);
//                 animTimer.stop();
//                 return;
//             }
//         }
//         return;
//     }
    
//     // ========================================================================
//     // СПЕЦИАЛЬНАЯ ОБРАБОТКА ПЛАВНОГО ЗАТУХАНИЯ (без таймера!)
//     // ========================================================================
    
//     if (ttState == TTState::FLASH_FADE) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 500;
        
//         if (elapsed < fadeDuration) {
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return;
//         } else {
//             ttState = TTState::DARK_1;
//             ttTimer.start(200);
//             return;
//         }
//     }
    
//     // ОБРАБОТКА ФИНАЛЬНОГО ЗАТУХАНИЯ (тоже без таймера!)
//     if (ttState == TTState::FADE_OUT) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 600;
        
//         if (elapsed < fadeDuration) {
//             // Плавная интерполяция от 255 до 0
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return; // Важно! Выходим здесь, чтобы обрабатывать на каждой итерации
//         } else {
//             // Затухание завершено
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::COMPLETE;
//             ttTimer.start(400);
//             return;
//         }
//     }
    
//     // ========================================================================
//     // ОБЫЧНАЯ ОБРАБОТКА ОСТАЛЬНЫХ СОСТОЯНИЙ
//     // ========================================================================
    
//     if (!ttTimer.tick()) return;
    
//     switch (ttState) {
//         case TTState::FLASH_START:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::Blue);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FLASH_HOLD;
//             ttTimer.start(1800);
//             break;
            
//         case TTState::FLASH_HOLD:
//             ttState = TTState::FLASH_FADE;
//             fadeStartTime = millis();
//             break;
            
//         case TTState::DARK_1:
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_1;
//             ttTimer.start(150);
//             break;
            
//         case TTState::BURST_1:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_2;
//             ttTimer.start(180);
//             break;
            
//         case TTState::DARK_2:
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_2;
//             ttTimer.start(250);
//             break;
            
//         case TTState::BURST_2:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_3;
//             ttTimer.start(220);
//             break;
            
//         case TTState::DARK_3:
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_3;
//             ttTimer.start(180);
//             break;
            
//         case TTState::BURST_3:
//             // Третий взрыв - самый яркий
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::White);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FADE_OUT;
//             fadeStartTime = millis(); // Запоминаем время начала затухания
//             // НЕ используем ttTimer.start() здесь!
//             break;
            
//         case TTState::COMPLETE:
//             // Восстановление и переход на следующий режим
//             FastLED.setBrightness(255);
//             delaySpeed = 80;
//             movieSpeed = 66.66;
//             ttState = TTState::RUNNING;
//             setRadChase2();
//             break;
            
//         default:
//             break;
//     }
// }

void runMovieTimeTravelReal() {
    // Основная анимация Time Travel (ускорение)
    if (ttState == TTState::RUNNING) {
        if (!animTimer.tick()) return;
        
        FastLED.clear();
        for (int j = 0; j <= 6; j++) {
            if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
                leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
            }
        }
        FastLED.show();
        
        animStep++;
        
        if (animStep >= NUM_LEDS) {
            delaySpeed *= 0.837;
            animTimer.setTime(delaySpeed);
            animStep = 0;
            
            if (delaySpeed < 1) {
                ttState = TTState::FLASH_START;
                ttTimer.start(50);
                animTimer.stop();
                return;
            }
        }
        return;
    }
    
    // ========================================================================
    // СПЕЦИАЛЬНАЯ ОБРАБОТКА ПЛАВНОГО ЗАТУХАНИЯ СИНЕГО
    // ========================================================================
    
    if (ttState == TTState::FLASH_FADE) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 500;
        
        if (elapsed < fadeDuration) {
            uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            // ВАЖНО: Перед переходом к темноте плавно выключаем синий
            ttState = TTState::DARK_1;
            fadeStartTime = millis(); // Сбрасываем таймер для финального затухания
            return; // НЕ вызываем ttTimer.start() здесь!
        }
    }
    
    // ОБРАБОТКА ТЕМНОТЫ С ПЛАВНЫМ ЗАТУХАНИЕМ СИНЕГО
    if (ttState == TTState::DARK_1) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 200; // 200 мс для затухания в темноту
        
        if (elapsed < fadeDuration) {
            // Плавное затухание от 180 до 0
            uint8_t brightness = map(elapsed, 0, fadeDuration, 180, 0);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            // Затухание завершено - полная темнота
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.setBrightness(255); // Восстанавливаем яркость для следующих эффектов
            FastLED.show();
            ttState = TTState::BURST_1;
            ttTimer.start(150);
            return;
        }
    }
    
    // ОБРАБОТКА ФИНАЛЬНОГО ЗАТУХАНИЯ БЕЛОГО
    if (ttState == TTState::FADE_OUT) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 600;
        
        if (elapsed < fadeDuration) {
            uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.setBrightness(255); // Восстанавливаем яркость
            FastLED.show();
            ttState = TTState::COMPLETE;
            ttTimer.start(400);
            return;
        }
    }
    
    // ========================================================================
    // ОБЫЧНАЯ ОБРАБОТКА ОСТАЛЬНЫХ СОСТОЯНИЙ
    // ========================================================================
    
    if (!ttTimer.tick()) return;
    
    switch (ttState) {
        case TTState::FLASH_START:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::FLASH_HOLD;
            ttTimer.start(1800);
            break;
            
        case TTState::FLASH_HOLD:
            ttState = TTState::FLASH_FADE;
            fadeStartTime = millis();
            break;
            
        // DARK_1 теперь обрабатывается выше, убираем отсюда
        
        case TTState::BURST_1:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::DARK_2;
            ttTimer.start(180);
            break;
            
        case TTState::DARK_2:
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.show();
            ttState = TTState::BURST_2;
            ttTimer.start(250);
            break;
            
        case TTState::BURST_2:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::DARK_3;
            ttTimer.start(220);
            break;
            
        case TTState::DARK_3:
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.show();
            ttState = TTState::BURST_3;
            ttTimer.start(180);
            break;
            
        case TTState::BURST_3:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::White);
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::FADE_OUT;
            fadeStartTime = millis();
            break;
            
        case TTState::COMPLETE:
            FastLED.setBrightness(255);
            delaySpeed = 80;
            movieSpeed = 66.66;
            ttState = TTState::RUNNING;
            setRadChase2();
            break;
            
        default:
            break;
    }
}


// void runMovieTimeTravelReal2() {
//     // Основная анимация Time Travel (ускорение) С ИСКРАМИ
//     if (ttState == TTState::RUNNING) {
//         if (!animTimer.tick()) return;
        
//         // Базовая анимация ускорения
//         FastLED.clear();
//         for (int j = 0; j <= 6; j++) {
//             if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
//                 leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
//             }
//         }
        
//         // ====================================================================
//         // ДОБАВЛЕНИЕ ЭФФЕКТА ИСКР (синие вспышки)
//         // ====================================================================
        
//         // Фаза 1: После ~40% цикла - первая искра
//         if (sparkCounter == 0 && delaySpeed < 70) {
//             unsigned long now = millis();
//             if (now - lastSparkTime > 100) { // Каждые 100 мс проверяем
//                 if (random(100) < 30) { // 30% шанс появления искры
//                     sparkPixel = random(NUM_LEDS);
//                     sparkBrightness = 255;
//                     sparkCounter = 1;
//                     lastSparkTime = now;
//                 }
//             }
//         }
        
//         // Фаза 2: После ~60% цикла - больше искр
//         else if (sparkCounter == 1 && delaySpeed < 40) {
//             unsigned long now = millis();
//             if (now - lastSparkTime > 80) { // Чаще проверяем
//                 if (random(100) < 50) { // 50% шанс
//                     sparkPixel = random(NUM_LEDS);
//                     sparkBrightness = 255;
//                     sparkCounter = 2;
//                     lastSparkTime = now;
//                 }
//             }
//         }
        
//         // Фаза 3: После ~80% цикла - частые искры
//         else if (sparkCounter == 2 && delaySpeed < 20) {
//             unsigned long now = millis();
//             if (now - lastSparkTime > 60) { // Ещё чаще
//                 if (random(100) < 70) { // 70% шанс
//                     sparkPixel = random(NUM_LEDS);
//                     sparkBrightness = 255;
//                     lastSparkTime = now;
//                 }
//             }
//         }
        
//         // Отрисовка искры (если она есть)
//         if (sparkPixel >= 0 && sparkBrightness > 0) {
//             // Синяя искра с затуханием
//             leds[sparkPixel] = CRGB(0, 50, sparkBrightness);
//             sparkBrightness -= 25; // Быстрое затухание
            
//             if (sparkBrightness <= 0) {
//                 sparkPixel = -1; // Искра погасла
//             }
//         }
        
//         FastLED.show();
        
//         // ====================================================================
//         // ПЕРЕХОД К ФИНАЛЬНОЙ ПОСЛЕДОВАТЕЛЬНОСТИ
//         // ====================================================================
        
//         animStep++;
        
//         if (animStep >= NUM_LEDS) {
//             delaySpeed *= 0.837;
//             animTimer.setTime(delaySpeed);
//             animStep = 0;
            
//             if (delaySpeed < 1) {
//                 ttState = TTState::FLASH_START;
//                 ttTimer.start(50);
//                 animTimer.stop();
                
//                 // Сброс переменных искр
//                 sparkCounter = 0;
//                 sparkPixel = -1;
//                 sparkBrightness = 0;
//                 return;
//             }
//         }
//         return;
//     }
    
//     // ========================================================================
//     // СПЕЦИАЛЬНАЯ ОБРАБОТКА ПЛАВНОГО ЗАТУХАНИЯ СИНЕГО
//     // ========================================================================
    
//     if (ttState == TTState::FLASH_FADE) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 500;
        
//         if (elapsed < fadeDuration) {
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return;
//         } else {
//             ttState = TTState::DARK_1;
//             fadeStartTime = millis();
//             return;
//         }
//     }
    
//     // ОБРАБОТКА ТЕМНОТЫ С ПЛАВНЫМ ЗАТУХАНИЕМ СИНЕГО
//     if (ttState == TTState::DARK_1) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 200;
        
//         if (elapsed < fadeDuration) {
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 180, 0);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return;
//         } else {
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::BURST_1;
//             ttTimer.start(150);
//             return;
//         }
//     }
    
//     // ОБРАБОТКА ФИНАЛЬНОГО ЗАТУХАНИЯ БЕЛОГО
//     if (ttState == TTState::FADE_OUT) {
//         unsigned long elapsed = millis() - fadeStartTime;
//         const unsigned long fadeDuration = 600;
        
//         if (elapsed < fadeDuration) {
//             uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
//             FastLED.setBrightness(brightness);
//             FastLED.show();
//             return;
//         } else {
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::COMPLETE;
//             ttTimer.start(400);
//             return;
//         }
//     }
    
//     // ========================================================================
//     // ОБЫЧНАЯ ОБРАБОТКА ОСТАЛЬНЫХ СОСТОЯНИЙ
//     // ========================================================================
    
//     if (!ttTimer.tick()) return;
    
//     switch (ttState) {
//         case TTState::FLASH_START:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::Blue);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FLASH_HOLD;
//             ttTimer.start(1800);
//             break;
            
//         case TTState::FLASH_HOLD:
//             ttState = TTState::FLASH_FADE;
//             fadeStartTime = millis();
//             break;
            
//         case TTState::BURST_1:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_2;
//             ttTimer.start(180);
//             break;
            
//         case TTState::DARK_2:
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_2;
//             ttTimer.start(250);
//             break;
            
//         case TTState::BURST_2:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::DARK_3;
//             ttTimer.start(220);
//             break;
            
//         case TTState::DARK_3:
//             digitalWrite(SINGLE_LED_PIN, LOW);
//             FastLED.clear();
//             FastLED.show();
//             ttState = TTState::BURST_3;
//             ttTimer.start(180);
//             break;
            
//         case TTState::BURST_3:
//             digitalWrite(SINGLE_LED_PIN, HIGH);
//             fill_solid(leds, NUM_LEDS, CRGB::White);
//             FastLED.setBrightness(255);
//             FastLED.show();
//             ttState = TTState::FADE_OUT;
//             fadeStartTime = millis();
//             break;
            
//         case TTState::COMPLETE:
//             FastLED.setBrightness(255);
//             delaySpeed = 80;
//             movieSpeed = 66.66;
//             ttState = TTState::RUNNING;
//             setRadChase2();
//             break;
            
//         default:
//             break;
//     }
// }

void runMovieTimeTravelReal2() {
    // Основная анимация Time Travel (ускорение) С ИСКРАМИ
    if (ttState == TTState::RUNNING) {
        if (!animTimer.tick()) return;
        
        // Базовая анимация ускорения
        FastLED.clear();
        for (int j = 0; j <= 6; j++) {
            if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
                leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
            }
        }
        
        // ====================================================================
        // ДОБАВЛЕНИЕ ЭФФЕКТА ИСКР (синие вспышки)
        // ====================================================================
        
        unsigned long now = millis();
        
        // Фаза 1: После ~40% цикла - первая искра
        if (sparkCounter == 0 && delaySpeed < 70) {
            // Проверяем: есть ли уже активная искра?
            if (sparkPixel < 0) { // Искра НЕ активна
                if (now - lastSparkTime > 100) { // Каждые 100 мс проверяем
                    if (random(100) < 30) { // 30% шанс появления искры
                        sparkPixel = random(NUM_LEDS);
                        sparkBrightness = 255;
                        lastSparkTime = now;
                        // НЕ переключаем sparkCounter здесь!
                    }
                }
            } else {
                // Искра активна, проверяем не погасла ли она
                if (sparkBrightness == 0 && sparkPixel < 0) {
                    // Искра полностью погасла, переходим к Фазе 2
                    sparkCounter = 1;
                }
            }
        }
        
        // Фаза 2: После ~60% цикла - больше искр
        else if (sparkCounter == 1 && delaySpeed < 40) {
            if (sparkPixel < 0) { // Искра НЕ активна
                if (now - lastSparkTime > 80) { // Чаще проверяем
                    if (random(100) < 50) { // 50% шанс
                        sparkPixel = random(NUM_LEDS);
                        sparkBrightness = 255;
                        lastSparkTime = now;
                    }
                }
            } else {
                // Искра активна, ждём когда погаснет
                if (sparkBrightness == 0 && sparkPixel < 0) {
                    // Искра погасла, можем перейти к Фазе 3
                    // Но не сразу, чтобы между искрами был промежуток
                    if (now - lastSparkTime > 80) {
                        sparkCounter = 2;
                    }
                }
            }
        }
        
        // Фаза 3: После ~80% цикла - частые искры
        else if (sparkCounter == 2 && delaySpeed < 20) {
            if (sparkPixel < 0) { // Искра НЕ активна
                if (now - lastSparkTime > 60) { // Ещё чаще
                    if (random(100) < 70) { // 70% шанс
                        sparkPixel = random(NUM_LEDS);
                        sparkBrightness = 255;
                        lastSparkTime = now;
                    }
                }
            }
        }
        
        // ====================================================================
        // ОТРИСОВКА И ЗАТУХАНИЕ ИСКРЫ
        // ====================================================================
        
        if (sparkPixel >= 0 && sparkBrightness > 0) {
            // Синяя искра с затуханием
            leds[sparkPixel] = CRGB(0, 50, sparkBrightness);
            
            if (sparkBrightness >= 25) {
                sparkBrightness -= 25; // Быстрое затухание
            } else {
                sparkBrightness = 0; // Гарантируем полное обнуление
                sparkPixel = -1; // Искра погасла
            }
        }
        
        FastLED.show();
        
        // ====================================================================
        // ПЕРЕХОД К ФИНАЛЬНОЙ ПОСЛЕДОВАТЕЛЬНОСТИ
        // ====================================================================
        
        animStep++;
        
        if (animStep >= NUM_LEDS) {
            delaySpeed *= 0.837;
            animTimer.setTime(delaySpeed);
            animStep = 0;
            
            if (delaySpeed < 1) {
                ttState = TTState::FLASH_START;
                ttTimer.start(50);
                animTimer.stop();
                
                // Сброс переменных искр
                sparkCounter = 0;
                sparkPixel = -1;
                sparkBrightness = 0;
                return;
            }
        }
        return;
    }
    
    // ========================================================================
    // СПЕЦИАЛЬНАЯ ОБРАБОТКА ПЛАВНОГО ЗАТУХАНИЯ СИНЕГО
    // ========================================================================
    
    if (ttState == TTState::FLASH_FADE) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 500;
        
        if (elapsed < fadeDuration) {
            uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            ttState = TTState::DARK_1;
            fadeStartTime = millis();
            return;
        }
    }
    
    // ОБРАБОТКА ТЕМНОТЫ С ПЛАВНЫМ ЗАТУХАНИЕМ СИНЕГО
    if (ttState == TTState::DARK_1) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 200;
        
        if (elapsed < fadeDuration) {
            uint8_t brightness = map(elapsed, 0, fadeDuration, 180, 0);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::BURST_1;
            ttTimer.start(150);
            return;
        }
    }
    
    // ОБРАБОТКА ФИНАЛЬНОГО ЗАТУХАНИЯ БЕЛОГО
    if (ttState == TTState::FADE_OUT) {
        unsigned long elapsed = millis() - fadeStartTime;
        const unsigned long fadeDuration = 600;
        
        if (elapsed < fadeDuration) {
            uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
            FastLED.setBrightness(brightness);
            FastLED.show();
            return;
        } else {
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::COMPLETE;
            ttTimer.start(400);
            return;
        }
    }
    
    // ========================================================================
    // ОБЫЧНАЯ ОБРАБОТКА ОСТАЛЬНЫХ СОСТОЯНИЙ
    // ========================================================================
    
    if (!ttTimer.tick()) return;
    
    switch (ttState) {
        case TTState::FLASH_START:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::Blue);
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::FLASH_HOLD;
            ttTimer.start(1800);
            break;
            
        case TTState::FLASH_HOLD:
            ttState = TTState::FLASH_FADE;
            fadeStartTime = millis();
            break;
            
        case TTState::BURST_1:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::DARK_2;
            ttTimer.start(180);
            break;
            
        case TTState::DARK_2:
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.show();
            ttState = TTState::BURST_2;
            ttTimer.start(250);
            break;
            
        case TTState::BURST_2:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::DARK_3;
            ttTimer.start(220);
            break;
            
        case TTState::DARK_3:
            digitalWrite(SINGLE_LED_PIN, LOW);
            FastLED.clear();
            FastLED.show();
            ttState = TTState::BURST_3;
            ttTimer.start(180);
            break;
            
        case TTState::BURST_3:
            digitalWrite(SINGLE_LED_PIN, HIGH);
            fill_solid(leds, NUM_LEDS, CRGB::White);
            FastLED.setBrightness(255);
            FastLED.show();
            ttState = TTState::FADE_OUT;
            fadeStartTime = millis();
            break;
            
        case TTState::COMPLETE:
            FastLED.setBrightness(255);
            delaySpeed = 80;
            movieSpeed = 66.66;
            ttState = TTState::RUNNING;
            setRadChase2();
            break;
            
        default:
            break;
    }
}

void runMovieTimeTravelSpeed() {
  // Получаем прогресс температуры (0.0 - 1.0)
  float tempProgress = getTempProgress();
  
  // ====================================================================
  // ФАЗА 1: УСКОРЕНИЕ (управляется температурой)
  // ====================================================================
  if (ttState == TTState::RUNNING) {
    // Вычисляем скорость на основе температуры
    // При 28°C: delaySpeed = 113 (медленно)
    // При ~60°C: delaySpeed = ~100 (всё ещё медленно)
    // При 80°C: delaySpeed = 0.5 (очень быстро)

    float baseSpeed = 113.0;
    float minSpeed = 0.5;

    // Применяем кастомную кривую ускорения
    // float easedProgress = customSpeedCurve(tempProgress); // применяем функцию кривой Безье
    float easedProgress = easeInCubic(tempProgress);

    // Интерполяция скорости
    delaySpeed = baseSpeed - (baseSpeed - minSpeed) * easedProgress;

    if (delaySpeed < minSpeed) delaySpeed = minSpeed;

    animTimer.setTime(delaySpeed);
    
    // Если таймер НЕ сработал - выходим
    if (!animTimer.tick()) return;
    
    // Базовая анимация ускорения
    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
      if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
        leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
      }
    }
    
    // ====================================================================
    // ЭФФЕКТ ИСКР (зависит от температуры)
    // ====================================================================
    unsigned long now = millis();
    
    // Частота искр зависит от прогресса температуры
    int sparkChance = (int)(easedProgress * 70); // 0-70% шанс
    unsigned long sparkDelay = 100 - (unsigned long)(easedProgress * 80); // 100мс -> 20мс
    
    if (sparkDelay < 20) sparkDelay = 20;
    
    if (sparkPixel < 0) { // Искра НЕ активна
      if (now - lastSparkTime > sparkDelay) {
        if (random(100) < sparkChance) {
          sparkPixel = random(NUM_LEDS);
          sparkBrightness = 255;
          lastSparkTime = now;
        }
      }
    }
    
    // Отрисовка и затухание искры
    if (sparkPixel >= 0 && sparkBrightness > 0) {
      leds[sparkPixel] = CRGB(0, 50, sparkBrightness);
      if (sparkBrightness >= 25) {
        sparkBrightness -= 25;
      } else {
        sparkBrightness = 0;
        sparkPixel = -1;
      }
    }
    
    FastLED.show();
    
    // ====================================================================
    // ПЕРЕХОД К ВСПЫШКЕ (при достижении 80°C)
    // ====================================================================
    animStep++;
    if (animStep >= NUM_LEDS) {
      animStep = 0;
      
      // Проверяем: достигли ли порога температуры?
      if (tempProgress >= 1.0) { // 80°C достигнуто!
        ttState = TTState::FLASH_START;
        ttTimer.start(50);
        animTimer.stop();
        
        // Сброс переменных искр
        sparkCounter = 0;
        sparkPixel = -1;
        sparkBrightness = 0;
        
        // ВАЖНО: Вызов timeTravel()
        timeCircuits.timeTravel();
        
        Serial.println(F("⚡⚡⚡ 80°C REACHED - TIME JUMP! ⚡⚡⚡"));
        return;
      }
    }
    return;
  }
  
  // ====================================================================
  // ФАЗА 2-N: ФИНАЛЬНАЯ ПОСЛЕДОВАТЕЛЬНОСТЬ (как в Real2)
  // ====================================================================
  
  // Плавное затухание синего
  if (ttState == TTState::FLASH_FADE) {
    unsigned long elapsed = millis() - fadeStartTime;
    const unsigned long fadeDuration = 500;
    if (elapsed < fadeDuration) {
      uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 180);
      FastLED.setBrightness(brightness);
      FastLED.show();
      return;
    } else {
      ttState = TTState::DARK_1;
      fadeStartTime = millis();
      return;
    }
  }
  
  // Темнота с затуханием
  if (ttState == TTState::DARK_1) {
    unsigned long elapsed = millis() - fadeStartTime;
    const unsigned long fadeDuration = 200;
    if (elapsed < fadeDuration) {
      uint8_t brightness = map(elapsed, 0, fadeDuration, 180, 0);
      FastLED.setBrightness(brightness);
      FastLED.show();
      return;
    } else {
      digitalWrite(SINGLE_LED_PIN, LOW);
      FastLED.clear();
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::BURST_1;
      ttTimer.start(150);
      return;
    }
  }
  
  // Финальное затухание белого
  if (ttState == TTState::FADE_OUT) {
    unsigned long elapsed = millis() - fadeStartTime;
    const unsigned long fadeDuration = 600;
    if (elapsed < fadeDuration) {
      uint8_t brightness = map(elapsed, 0, fadeDuration, 255, 0);
      FastLED.setBrightness(brightness);
      FastLED.show();
      return;
    } else {
      digitalWrite(SINGLE_LED_PIN, LOW);
      FastLED.clear();
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::COMPLETE;
      ttTimer.start(400);
      return;
    }
  }
  
  // Остальные состояния (взрывы)
  if (!ttTimer.tick()) return;
  
  switch (ttState) {
    case TTState::FLASH_START:
      digitalWrite(SINGLE_LED_PIN, HIGH);
      fill_solid(leds, NUM_LEDS, CRGB::Blue);
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::FLASH_HOLD;
      ttTimer.start(1800);
      break;
      
    case TTState::FLASH_HOLD:
      ttState = TTState::FLASH_FADE;
      fadeStartTime = millis();
      break;
      
    case TTState::BURST_1:
      digitalWrite(SINGLE_LED_PIN, HIGH);
      fill_solid(leds, NUM_LEDS, CRGB(150, 180, 255));
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::DARK_2;
      ttTimer.start(180);
      break;
      
    case TTState::DARK_2:
      digitalWrite(SINGLE_LED_PIN, LOW);
      FastLED.clear();
      FastLED.show();
      ttState = TTState::BURST_2;
      ttTimer.start(250);
      break;
      
    case TTState::BURST_2:
      digitalWrite(SINGLE_LED_PIN, HIGH);
      fill_solid(leds, NUM_LEDS, CRGB(180, 200, 255));
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::DARK_3;
      ttTimer.start(220);
      break;
      
    case TTState::DARK_3:
      digitalWrite(SINGLE_LED_PIN, LOW);
      FastLED.clear();
      FastLED.show();
      ttState = TTState::BURST_3;
      ttTimer.start(180);
      break;
      
    case TTState::BURST_3:
      digitalWrite(SINGLE_LED_PIN, HIGH);
      fill_solid(leds, NUM_LEDS, CRGB::White);
      FastLED.setBrightness(255);
      FastLED.show();
      ttState = TTState::FADE_OUT;
      fadeStartTime = millis();
      break;
      
    case TTState::COMPLETE:
      FastLED.setBrightness(255);
      delaySpeed = 80;
      movieSpeed = 66.66;
      ttState = TTState::RUNNING;
      speedAnimationActive = false; // Сброс флага
      setRadChase2();
      break;
      
    default:
      break;
  }
}

