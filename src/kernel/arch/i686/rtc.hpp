#pragma once

#include <stdint.h>
#include <stdbool.h>

class RTC {
public:
    struct RTCTime {
        uint8_t sec, min, hour;
        uint8_t day, mon, year;
        uint8_t cen;
    };

    static void Init(bool binary, bool time_24hr);
    static void GetTime(RTCTime& time);
    static void SetTime(const RTCTime& time);
    static void LogTime(const char* module, const RTCTime& time);
};