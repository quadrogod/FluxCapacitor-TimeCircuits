//
// Created by Alexey Molchanov on 12.11.2025.
//

#include "Config.h"
#include "AnimationManager.h"

AnimationManager::AnimationManager(ISensor* sens, ILogger* log)
    : sensor(sens), logger(log) {}

void AnimationManager::init() {
    FastLED.addLeds<WS2812B, DATA_RING_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_POWER_LIMIT_MA);
    FastLED.clear();
    FastLED.show();
    logger->println(F("Animation Manager Initialized"));
}

void AnimationManager::update() {
    // Обновляем сенсор
    if (sensor) {
        sensor->update();
        // Автоматический запуск анимации скорости
        // if (sensor->isActive() && currentAnimation == AnimationType::OFF) {
        //     setAnimation(AnimationType::TIME_TRAVEL);
        // }
    }

    switch (currentAnimation) {
        case AnimationType::SLOW_FLOW:      runSlowFlow();break;
        case AnimationType::MIDDLE_FLOW:    runMiddleFlow();break;
        case AnimationType::FAST_FLOW:      runFastFlow();break;
        case AnimationType::MOVIE_FLOW:     runMovieFlow();break;
        case AnimationType::MOVIE_FLOW_REAL:runMovieFlowReal();break;
        case AnimationType::RAINBOW_FLOW:   runRainbowFlow();break;
        case AnimationType::TIME_TRAVEL:    runTimeTravel();break;
        case AnimationType::OFF:
        default:
            runOff();
            break;
    }
}

void AnimationManager::resetAnimationState() {
    currentAnimation = AnimationType::OFF;
    animTimer.stop();
    ttTimer.stop();
    hueTimer.stop();
    ttState = TTState::RUNNING;
    animStep = 0;
    animSubStep = 0;
}

void AnimationManager::setAnimation(AnimationType anim) {
    if (currentAnimation == anim) return;
    //
    resetAnimationState();
    currentAnimation = anim;
    currentAnimationConfig = animations[static_cast<int>(anim)];
    animTimer.setTime(currentAnimationConfig.delay);
    animTimer.start();
    if (anim == AnimationType::RAINBOW_FLOW) {
        hueTimer.setTime(5);
        hueTimer.start();
    }
    logger->println(currentAnimationConfig.activatedMessage);
}

void AnimationManager::runOff() {
    FastLED.clear();
    FastLED.show();
}

void AnimationManager::runSlowFlow() {
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

void AnimationManager::runMiddleFlow() {
    if (!animTimer.tick()) return;

    FastLED.clear();
    if (animStep < NUM_LEDS) {
        leds[animStep] = CHSV(28, 200, 120);
        if (animStep > 0) leds[animStep - 1] = CHSV(28, 200, 30);
        if (animStep < NUM_LEDS - 1) leds[animStep + 1] = CHSV(28, 200, 30);
    }
    FastLED.show();

    animStep++;
    if (animStep >= NUM_LEDS_ON_LINE) animStep = 0;
}

void AnimationManager::runFastFlow() {
    if (!animTimer.tick()) return;

    FastLED.clear();
    int base = animStep * 2;
    if (base < NUM_LEDS - 1) {
        leds[base] = leds[base + 1] = CHSV(28, 120, 100);
    }
    FastLED.show();

    animStep++;
    if (animStep >= NUM_LEDS_ON_LINE / 2) animStep = 0;
}

void AnimationManager::runMovieFlow() {
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

void AnimationManager::runMovieFlowReal() {
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

void AnimationManager::runRainbowFlow() {
    // Обновление hue каждые 5 мс
    if (hueTimer.tick()) hue++;

    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CHSV(hue + i * 10, 255, 200);
    }
    FastLED.show();
}

void AnimationManager::runTimeTravel() {
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
        if (sparkCounter == 0 && currentAnimationConfig.delay < 70) {
            // Проверяем: есть ли уже активная искра?
            if (sparkPixel < 0) { // Искра НЕ активна
                if (now - lastSparkTime > 100) { // Каждые 100 мс проверяем
                    if (random(100) < 30) { // 30% шанс появления искры
                        sparkPixel = random(NUM_LEDS);
                        sparkBrightness = 255;
                        lastSparkTime = now;
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
        else if (sparkCounter == 1 && currentAnimationConfig.delay < 40) {
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
        else if (sparkCounter == 2 && currentAnimationConfig.delay < 20) {
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
            currentAnimationConfig.delay *= 0.837;
            animTimer.setTime(currentAnimationConfig.delay);
            animStep = 0;

            if (currentAnimationConfig.delay < 1) {
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
            // delaySpeed = 80;
            // movieSpeed = 66.66;
            ttState = TTState::RUNNING;
            setAnimation(AnimationType::SLOW_FLOW);
            break;

        default:
            break;
    }
}