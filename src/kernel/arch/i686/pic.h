#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char* Name;
    bool (*Probe)();
    void (*Initialize)(uint8_t, uint8_t, bool);
    void (*Disable)();
    void (*SendEOI)(int);
    void (*Mask)(int);
    void (*Unmask)(int);
} PIC_Driver;
