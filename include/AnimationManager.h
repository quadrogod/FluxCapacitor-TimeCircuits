//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_ANIMATIONMANAGER_H
#define FLUXCAPACITOR_TIMECIRCUITS_ANIMATIONMANAGER_H

#include <Arduino.h>
#include <FastLED.h>
#include <GTimer.h>
#include "Config.h"
#include "../src/core/Sensor/ISensor.h"
#include "../src/core/Logger/ILogger.h"
#include "ITimeTravelValidator.h"

class AnimationManager {
    public:
        struct AnimationConfig {
            const char* activatedMessage;
            float delay;
        };

        enum class AnimationType {
            OFF,
            LOW_POWER,
            SLOW_FLOW,
            MIDDLE_FLOW,
            FAST_FLOW,
            MOVIE_FLOW,
            MOVIE_FLOW_REAL,
            RAINBOW_FLOW,
            TIME_TRAVEL,
            TIME_TRAVEL_REAL
        };


        // количество AnimationType
        AnimationConfig animations[10] = {
            /* OFF */ { "OFF Mode activated", 0 },
            /* LOW_POWER */ { "Low Power activated", 66.66 },
            /* SLOW_FLOW */ { "Slow flow activated", 66.66 },
            /* MIDDLE_FLOW */ { "Middle flow activated", 66.66 },
            /* FAST_FLOW */ { "Fast flow activated", 66.66 },
            /* MOVIE_FLOW */ { "Movie flow activated", 22.97 },
            /* MOVIE_FLOW_REAL */ { "Movie flow real activated", 34.45 },
            /* RAINBOW_FLOW */ { "Rainbow flow activated", 66.66 },
            /* TIME_TRAVEL */ { "Time Travel activated", 66.66 }, // 113
            /* TIME_TRAVEL_REAL */ { "Time Travel Real activated", 66.66 }, // 113
        };

        AnimationManager(ISensor* sens, ILogger* log);
        void init();
        void update();
        void setAnimation(AnimationType anim);
        AnimationType getCurrentAnimation() const { return currentAnimation; }

        // Доступ к LED массиву для внешних манипуляций если нужно
        // CRGB* getLEDs() { return leds; }
        // int getLEDCount() const { return NUM_LEDS; }

        void setTimeTravelValidator(ITimeTravelValidator* validator) {
            timeTravelValidator = validator;
        }

        void toggleColor();  // Переключение оттенка

    private:
        // FastLED
        CRGB leds[NUM_LEDS];
        uint8_t hue = 0;
        // Текущая анимация
        AnimationType currentAnimation = AnimationType::OFF;
        AnimationConfig currentAnimationConfig;
        // Зависимости
        ISensor* sensor;
        ILogger* logger;
        ITimeTravelValidator* timeTravelValidator;

        // Таймеры для анимаций
        GTimer<millis> animTimer;
        GTimer<millis> hueTimer; // для rainbow эффекта

        int animStep = 0;
        int animSubStep = 0;

        int sparkCounter = 0; // Счётчик для отслеживания фазы искр
        unsigned long lastSparkTime = 0; // Время последней искры
        int sparkPixel = -1;            // Текущий пиксель искры
        uint8_t sparkBrightness = 0;    // Яркость искры для затухания

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

        TTState ttState = TTState::RUNNING;
        GTimer<millis> ttTimer;
        unsigned long fadeStartTime = 0; // Для отслеживания времени фейда
        bool timeTravelCompleted = false;  // Флаг завершения цикла анимации

        bool useWarmColor = true; // теплый свет (как в оригинале) или холодный
        //
        void resetAnimationState();
        void runOff();
        void runLowPower();
        void runSlowFlow();
        void runMiddleFlow();
        void runFastFlow();
        void runMovieFlow();
        void runMovieFlowReal();
        void runRainbowFlow();
        void runTimeTravel();
        void runTimeTravelReal();
        //
        void drawSlowFlow();
        void drawAlertLED();
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_ANIMATIONMANAGER_H