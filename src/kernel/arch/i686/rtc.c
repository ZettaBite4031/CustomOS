#include "rtc.h"
#include "io.h"
#include "timer.h"
#include "debug.h"

#define CMOS_ADDR 0x70
#define CMOS_DATA 0x71

static inline uint8_t cmos_read(uint8_t reg) {
    i686_OutB(CMOS_ADDR, reg | 0x80);
    return i686_InB(CMOS_DATA);
}

static inline void cmos_write(uint8_t reg, uint8_t val) {
    i686_OutB(CMOS_ADDR, reg | 0x80);
    i686_OutB(CMOS_DATA, val);
}

static uint8_t bcd_to_bin(uint8_t b) { return (b & 0x0F) + ((b >> 4) * 10); }
static uint8_t bin_to_bcd(uint8_t b) { return ((b/10)<<4) | (b%10); }

void RTC_Init(bool binary_format, bool time_24hr) {
    // set data format
    uint8_t status_reg = cmos_read(0x0B); 
    if (time_24hr)      status_reg |= 1 << 1; // Enable 24 format
    if (binary_format)  status_reg |= 1 << 2; // Enable binary format
    cmos_write(0x0B, status_reg);

    // set interrupt rate
    // See MC146818 docs for rate control, set up 256Hz
    uint8_t data = cmos_read(0x0A);
    data = (data & 0xF0) | 1;
    cmos_write(0x0A, data);

    // enable interrupts
    uint8_t original = cmos_read(0x8B);
    cmos_write(0x8B, original | 0x40);
}

void RTC_GetTime(rtc_time_t* t) {

    while (cmos_read(0x0A) & 0x80) sleep(250);

    uint8_t sec  = cmos_read(0x00);
    uint8_t min  = cmos_read(0x02);
    uint8_t hr   = cmos_read(0x04);
    uint8_t day  = cmos_read(0x07);
    uint8_t mon  = cmos_read(0x08);
    uint8_t yr   = cmos_read(0x09);
    uint8_t cen  = cmos_read(0x32);

    uint8_t ctrl = cmos_read(0x0B);
    bool bcd     = !(ctrl & (1<<2));

    if (bcd) {
        sec = bcd_to_bin(sec);
        min = bcd_to_bin(min);
        hr  = bcd_to_bin(hr & 0x7F);
        day = bcd_to_bin(day);
        mon = bcd_to_bin(mon);
        yr  = bcd_to_bin(yr);
        cen  = bcd_to_bin(cen);
    } else {
        hr &= 0x7F;
    }

    t->sec  = sec;
    t->min  = min;
    t->hour = hr;
    t->day  = day;
    t->mon  = mon;
    t->year = yr;
    t->cen  = cen;
}

void RTC_SetTime(const rtc_time_t* t) {
    uint8_t ctrl = cmos_read(0x0B);
    cmos_write(0x0B, ctrl | 0x80);

    bool bcd = !(ctrl & (1<<2));

    cmos_write(0x00, bcd ? bin_to_bcd(t->sec)  : t->sec);
    cmos_write(0x02, bcd ? bin_to_bcd(t->min)  : t->min);
    cmos_write(0x04, bcd ? bin_to_bcd(t->hour) : t->hour);
    cmos_write(0x07, bcd ? bin_to_bcd(t->day)  : t->day);
    cmos_write(0x08, bcd ? bin_to_bcd(t->mon)  : t->mon);
    cmos_write(0x09, bcd ? bin_to_bcd(t->year) : t->year);
    cmos_write(0x32, bcd ? bin_to_bcd(t->cen)  : t->cen);

    cmos_write(0x0B, ctrl & ~0x80);
}

void RTC_LogTime(const char* module, const rtc_time_t* t) {
    const char* month_str;
    switch(t->mon) {
        case 1: month_str = "January"; break;
        case 2: month_str = "Febuary"; break;
        case 3: month_str = "March"; break;
        case 4: month_str = "April"; break;
        case 5: month_str = "May"; break;
        case 6: month_str = "June"; break;
        case 7: month_str = "July"; break;
        case 8: month_str = "August"; break;
        case 9: month_str = "September"; break;
        case 10: month_str = "October"; break;
        case 11: month_str = "November"; break;
        case 12: month_str = "December"; break;
        default: month_str = "unknown"; break;
    }

    LogInfo(module, "Current RTC Time: %d %s, %d%d %d:%d:%d", t->day, month_str, t->cen, t->year, t->hour, t->min, t->sec);
}