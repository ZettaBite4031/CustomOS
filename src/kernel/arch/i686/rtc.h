#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct rtc_time {
    uint8_t sec, min, hour;
    uint8_t day, mon, year;
    uint8_t cen;
} rtc_time_t;

void RTC_Init(bool binary_format, bool time_24hr);
void RTC_GetTime(rtc_time_t* t);
void RTC_SetTime(const rtc_time_t* t);
void RTC_LogTime(const char* module, const rtc_time_t* t);