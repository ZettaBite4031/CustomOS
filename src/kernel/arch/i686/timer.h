#pragma once
#include <stdint.h>

extern volatile uint32_t pit_ticks;

void PIT_Init(uint32_t freq_hz);
void sleep(uint32_t ms);