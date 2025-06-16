#pragma once

#include <stdint.h>
#include <stddef.h>

namespace RTC {
    struct Time {
        uint8_t sec, min, hour;
        uint8_t day, mon, year;
        uint8_t cen;
    };

    void Init(bool binary, bool time_24hr);
    void GetTime(Time& time);
    void SetTime(const Time& time);
    void LogTime(const char* module, const Time& time);
}