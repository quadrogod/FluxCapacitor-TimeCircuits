//
// Created by Alexey Molchanov on 12.11.2025.
//

#ifndef FLUXCAPACITOR_TIMECIRCUITS_DS3231RTCPROVIDER_H
#define FLUXCAPACITOR_TIMECIRCUITS_DS3231RTCPROVIDER_H

#include <Arduino.h>
#include "IRTCProvider.h"

#ifdef USE_RTC_DS3231
#include <RTClib.h>
#endif

class DS3231RTCProvider : public IRTCProvider {
private:
#ifdef USE_RTC_DS3231
    RTC_DS3231 rtc;
    uint8_t lastMinute;
    bool available;
#endif

public:
    DS3231RTCProvider()
#ifdef USE_RTC_DS3231
        : lastMinute(0), available(false)
#endif
    {}

    bool init() override {
        #ifdef USE_RTC_DS3231
            if (!rtc.begin()) {
                available = false;
                return false;
            }

            if (rtc.lostPower()) {
                rtc.adjust(DateTime(2020, 1, 1, 0, 0, 0));
            }

            DateTime now = rtc.now();
            lastMinute = now.minute();
            available = true;
            return true;
        #else
            return false;
        #endif
    }

    bool isAvailable() override {
        #ifdef USE_RTC_DS3231
            return available;
        #else
            return false;
        #endif
    }

    TCDateTime getCurrentTime() override {
        TCDateTime dt;
        #ifdef USE_RTC_DS3231
            if (!available) return dt;

            DateTime now = rtc.now();
            dt.y = now.year();
            dt.m = now.month();
            dt.d = now.day();
            dt.h = now.hour();
            dt.min = now.minute();
            dt.valid = true;
        #endif
        return dt;
    }

    void setTime(const TCDateTime& dt) override {
        #ifdef USE_RTC_DS3231
            if (!available) return;
            rtc.adjust(DateTime(dt.y, dt.m, dt.d, dt.h, dt.min, 0));
        #endif
    }

    bool hasMinuteTick() override {
        #ifdef USE_RTC_DS3231
            if (!available) return false;

            DateTime now = rtc.now();
            uint8_t currentMinute = now.minute();

            if (currentMinute != lastMinute) {
                lastMinute = currentMinute;
                return true;
            }
        #endif
        return false;
    }

    bool isSupported(const TCDateTime& dt) override {
        // у датчика ds1307 года хранятся как 00 - 99, но календарь учитывающий високосные года от 2000 года,
        // поэтому если кто-то решит использовать время как календарь, то модуль пригодится, для всех остальных случаев,
        // мы просто будем считать тики изменения минуты, не более.
        return (dt.y >= 2000) && (dt.y <= 2099);
    }

    DateTime now() override {
        #ifdef USE_RTC_DS3231
            if (!available) return DateTime(2000, 1, 1, 0, 0, 0);
        #endif
        return rtc.now();
    }
};

#endif //FLUXCAPACITOR_TIMECIRCUITS_DS3231RTCPROVIDER_H