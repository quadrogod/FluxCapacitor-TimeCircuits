//
// Created by Alexey Molchanov on 12.11.2025.
//

#include "Config.h"
#include "EaseUtils.h"
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

void AnimationManager::toggleColor() {
    useWarmColor = !useWarmColor;
}

void AnimationManager::update() {
    // Обновляем сенсор
    if (sensor) {
        sensor->update();

        // Сброс флага блокировки при падении температуры ниже resetThreshold
        if (!sensor->isActive() && timeTravelCompleted) {
            timeTravelCompleted = false;
            logger->println(F("Temperature reset. Ready for new cycle."));
        }

        // Автоматический запуск анимации скорости
        if (sensor->isActive() && currentAnimation != AnimationType::TIME_TRAVEL_REAL && !timeTravelCompleted) {
            setAnimation(AnimationType::TIME_TRAVEL_REAL);
        }
    }

    switch (currentAnimation) {
        case AnimationType::LOW_POWER:          runLowPower();break;
        case AnimationType::SLOW_FLOW:          runSlowFlow();break;
        case AnimationType::MIDDLE_FLOW:        runMiddleFlow();break;
        case AnimationType::FAST_FLOW:          runFastFlow();break;
        case AnimationType::MOVIE_FLOW:         runMovieFlow();break;
        case AnimationType::MOVIE_FLOW_REAL:    runMovieFlowReal();break;
        case AnimationType::RAINBOW_FLOW:       runRainbowFlow();break;
        case AnimationType::TIME_TRAVEL:        runTimeTravel();break;
        case AnimationType::TIME_TRAVEL_REAL:   runTimeTravelReal();break;
        case AnimationType::OFF:
        default:
            runOff();
            break;
    }
}

void AnimationManager::resetAnimationState() {
    digitalWrite(SINGLE_LED_PIN, LOW);
    currentAnimation = AnimationType::OFF;
    animTimer.stop();
    ttTimer.stop();
    hueTimer.stop();
    ttState = TTState::RUNNING;
    animStep = 0;
    animSubStep = 0;
}

void AnimationManager::setAnimation(AnimationType anim) {
    if (currentAnimation == anim) {
        toggleColor();
        return;
    }
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

void AnimationManager::runLowPower() {
    if (!animTimer.tick()) return;

    FastLED.clear();

    // ====================================================================
    // АНИМАЦИЯ ЛЕНТЫ (как в runSlowFlow)
    // ====================================================================
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0) {
            if (useWarmColor) {
                leds[animStep - j] = CHSV(22, 200, 20 + j * 30);
            } else {
                leds[animStep - j] = CHSV(28, 120, 20 + j * 30);
            }
        }
    }

    FastLED.show();

    // ====================================================================
    // УПРАВЛЕНИЕ СИГНАЛЬНЫМ СВЕТОДИОДОМ
    // ====================================================================
    // NUM_LEDS = 22, но реальных пикселей 10
    // Цикл занимает ~800мс паузу (когда animStep >= 10)
    // Диод должен:
    // - Гореть на полную яркость в начале (animStep 0-10)
    // - Плавно тухнуть в конце цикла (animStep 10-22)

    if (animStep == 0) {
        digitalWrite(SINGLE_LED_PIN, HIGH);
    }
    if (animStep == 9) {
        digitalWrite(SINGLE_LED_PIN, LOW);
    }

    if (animStep < NUM_LEDS_ON_LINE) {
        // Фаза активности ленты — диод горит на полную
        // analogWrite(SINGLE_LED_PIN, HIGH);
        // digitalWrite(SINGLE_LED_PIN, HIGH);
    } else {
        // Фаза паузы ленты — диод плавно гаснет
        // Рассчитываем прогресс затухания: от 255 до 0
        int fadeSteps = NUM_LEDS - NUM_LEDS_ON_LINE; // 22 - 10 = 12 шагов
        int currentFadeStep = animStep - NUM_LEDS_ON_LINE; // 0..11

        // Линейное затухание от 255 до 0
        int brightness = map(currentFadeStep, 0, fadeSteps, 255, 0);
        if (brightness < 0) brightness = 0;

        // analogWrite(SINGLE_LED_PIN, brightness);
        // digitalWrite(SINGLE_LED_PIN, HIGH);
    }

    // ====================================================================
    // ПЕРЕХОД К СЛЕДУЮЩЕМУ ШАГУ
    // ====================================================================
    animStep++;
    if (animStep >= NUM_LEDS) {
        animStep = 0;
        // Гарантируем, что диод полностью погас перед новым циклом
        // analogWrite(SINGLE_LED_PIN, 0);
        digitalWrite(SINGLE_LED_PIN, LOW);
        // delay(20); // Короткая пауза для визуальной чёткости
    }
}

void AnimationManager::runSlowFlow() {
    if (!animTimer.tick()) return;

    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0) {
            if (useWarmColor) {
                leds[animStep - j] = CHSV(22, 200, 20 + j * 30);
            } else {
                leds[animStep - j] = CHSV(28, 120, 20 + j * 30);
            }
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
        if (useWarmColor) {
            leds[animStep] = CHSV(22, 200, 100);
            if (animStep > 0) leds[animStep - 1] = CHSV(22, 200, 30);
            if (animStep < NUM_LEDS - 1) leds[animStep + 1] = CHSV(22, 200, 30);
        } else {
            leds[animStep] = CHSV(28, 120, 100);
            if (animStep > 0) leds[animStep - 1] = CHSV(28, 120, 30);
            if (animStep < NUM_LEDS - 1) leds[animStep + 1] = CHSV(28, 120, 30);
        }
    }
    FastLED.show();

    animStep++;
    if (animStep >= NUM_LEDS_ON_LINE) animStep = 0;
}

void AnimationManager::runFastFlow() {
    if (!animTimer.tick()) return;

    FastLED.clear();
    int idx = animStep * 2;
    if (idx < NUM_LEDS - 1) {
        if (useWarmColor) {
            leds[idx] = leds[idx + 1] = CHSV(22, 200, 100);
        } else {
            leds[idx] = leds[idx + 1] = CHSV(28, 120, 100);
        }
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
        // leds[idx] = leds[idx + 1] = CHSV(22, 200, 100);
        if (useWarmColor) {
            leds[idx] = leds[idx + 1] = CHSV(22, 200, 100);
        } else {
            leds[idx] = leds[idx + 1] = CHSV(28, 120, 100);
        }
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
        // leds[idx] = CHSV(22, 200, 100);
        if (useWarmColor) {
            leds[idx] = CHSV(22, 200, 100);
        } else {
            leds[idx] = CHSV(28, 120, 100);
        }
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
                if (useWarmColor) {
                    leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
                } else {
                    leds[animStep - j] = CHSV(28, 120, 60 + j * 30);
                }
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
            ttState = TTState::RUNNING;
            // setAnimation(AnimationType::SLOW_FLOW);
            setAnimation(AnimationType::LOW_POWER);
            break;

        default:
            break;
    }
}

void AnimationManager::runTimeTravelReal() {
    // ====================================================================
    // ПРОВЕРКА АКТИВНОСТИ ДАТЧИКА
    // ====================================================================
    // Если датчик неактивен (температура упала ниже resetThreshold),
    // и мы ещё в фазе ускорения, останавливаем анимацию
    if (!sensor->isActive() && ttState == TTState::RUNNING) {
        setAnimation(AnimationType::SLOW_FLOW);
        return;
    }

    // ====================================================================
    // ОБРАБОТКА ФИНАЛЬНОЙ ПОСЛЕДОВАТЕЛЬНОСТИ (игнорируем датчик)
    // ====================================================================
    // Если мы уже запустили вспышку, датчик больше не влияет на анимацию
    if (ttState != TTState::RUNNING) {
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
                ttState = TTState::RUNNING;
                timeTravelCompleted = true;  // флаг блокировки повторной анимации если данные сенсора сохранили состояние
                // setAnimation(AnimationType::SLOW_FLOW);
                setAnimation(AnimationType::LOW_POWER);
                logger->println(F("Time travel completed. Waiting for sensor reset..."));
                break;

            default:
                break;
        }
        return;
    }

    // ====================================================================
    // ФАЗА УСКОРЕНИЯ (управляется температурой)
    // ====================================================================
    // Получаем прогресс температуры (0.0 - 1.0)
    float tempProgress = sensor->getProgress();

    // ИСПРАВЛЕНИЕ: Фиксированная базовая скорость!
    // const float baseSpeed = 113.0;  // НЕ берём из currentAnimationConfig.delay!
    const float baseSpeed = 66.66;  // что бы не отличалось от SLOW_FLOW
    const float minSpeed = 0.5;

    // Применяем функцию кривой (медленный старт, быстрое ускорение)
    float easedProgress = easeInCubic(tempProgress);

    // Интерполяция скорости: от 113 до 0.5
    float newDelay = baseSpeed - (baseSpeed - minSpeed) * easedProgress;
    if (newDelay < minSpeed) newDelay = minSpeed;

    // Устанавливаем новую скорость
    animTimer.setTime(newDelay);

    // Если таймер НЕ сработал - выходим
    if (!animTimer.tick()) return;

    // ====================================================================
    // БАЗОВАЯ АНИМАЦИЯ УСКОРЕНИЯ
    // ====================================================================
    FastLED.clear();
    for (int j = 0; j <= 6; j++) {
        if (animStep - j >= 0 && animStep - j < NUM_LEDS) {
            if (useWarmColor) {
                leds[animStep - j] = CHSV(22, 200, 60 + j * 30);
            } else {
                leds[animStep - j] = CHSV(28, 120, 60 + j * 30);
            }
        }
    }

    // ====================================================================
    // ЭФФЕКТ ИСКР (зависит от температуры)
    // ====================================================================
    unsigned long now = millis();

    // Частота искр зависит от прогресса температуры
    int sparkChance = (int)(easedProgress * 70);  // 0-70% шанс
    unsigned long sparkDelay = 100 - (unsigned long)(easedProgress * 80);  // 100мс -> 20мс
    if (sparkDelay < 20) sparkDelay = 20;

    if (sparkPixel < 0) {  // Искра НЕ активна
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
    // ПЕРЕХОД К ВСПЫШКЕ (при достижении triggerThreshold)
    // ====================================================================
    animStep++;
    if (animStep >= NUM_LEDS) {
        animStep = 0;

        // ИСПРАВЛЕНИЕ: Используем shouldTrigger() вместо tempProgress >= 1.0
        if (sensor->shouldTrigger()) {
            bool canFullTravel = (timeTravelValidator != nullptr) &&
                                  timeTravelValidator->canPerformFullTimeTravel();
            if (canFullTravel) {
                ttState = TTState::FLASH_START;
                ttTimer.start(50);
                animTimer.stop();

                // Сброс переменных искр
                sparkCounter = 0;
                sparkPixel = -1;
                sparkBrightness = 0;

                // Вызываем путешествие во времени
                timeTravelValidator->performTimeTravel();
                logger->println(F("⚡⚡⚡ 88MP/h REACHED - TIME JUMP! ⚡⚡⚡"));
            } else {
                // ===== ОЖИДАНИЕ УСТАНОВКИ ВРЕМЁН =====
                // logger->println(F("⚡ 88MP/h reached! Waiting for times to be set..."));
                // лог не пишем, иначе срабатывает постоянно, надо что-то придумать тоже, что бы один раз писал
            }
            //
            return;
        }
    }
}
