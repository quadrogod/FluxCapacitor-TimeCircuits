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
        case AnimationType::SLOW_FLOW:
            runSlowFlow();
            break;

        case AnimationType::MIDDLE_FLOW:
            runMiddleFlow();
            break;

        case AnimationType::FAST_FLOW:
            runFastFlow();
            break;

        case AnimationType::MOVIE_FLOW:
            runMovieFlow();
            break;

        case AnimationType::MOVIE_FLOW_REAL:
            runMovieFlowReal();
            break;

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