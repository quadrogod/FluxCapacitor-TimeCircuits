#include <Arduino.h>
#include "Globals.h"
#include "SerialHandler.h"

void handleSerial() {
    if (!Serial.available()) return;
    
    char k = Serial.read();
    
    // Очистка буфера от лишних символов (например \n)
    while (Serial.available()) Serial.read();
    
    Serial.print(F("Received: "));
    Serial.println(k);
    
    switch (k) {
        case '1':
            animationManager.setAnimation(AnimationManager::AnimationType::SLOW_FLOW);
            break;

        case '2':
            animationManager.setAnimation(AnimationManager::AnimationType::MIDDLE_FLOW);
            break;

        case '3':
            animationManager.setAnimation(AnimationManager::AnimationType::FAST_FLOW);
            break;

        case '4':
            animationManager.setAnimation(AnimationManager::AnimationType::MOVIE_FLOW);
            break;

        case '5':
            animationManager.setAnimation(AnimationManager::AnimationType::MOVIE_FLOW_REAL);
            break;

        case '6':
            animationManager.setAnimation(AnimationManager::AnimationType::TIME_TRAVEL);
            break;

        case '7':
            animationManager.setAnimation(AnimationManager::AnimationType::LOW_POWER);
            break;

        case '8':
            animationManager.setAnimation(AnimationManager::AnimationType::RAINBOW_FLOW);
            break;

        case '0':
            animationManager.setAnimation(AnimationManager::AnimationType::OFF);
            break;
            
        default:
            Serial.println(F("Unknown command"));
            Serial.println(k);
            break;
    }
}
