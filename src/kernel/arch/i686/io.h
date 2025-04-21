#pragma once
#include <stdint.h>
#include <zosdefs.h>


void EXTERN i686_OutB(uint16_t port, uint8_t value);
uint8_t EXTERN i686_InB(uint16_t port);
void EXTERN i686_OutW(uint16_t port, uint16_t value);
uint16_t EXTERN i686_InW(uint16_t port);
void EXTERN i686_OutL(uint16_t port, uint32_t value);
uint32_t EXTERN i686_InL(uint16_t port);

void EXTERN i686_EnableInterrupts();
void EXTERN i686_DisableInterrupts();

// halt the device during a panic state.
void EXTERN i686_PANIC();

void i686_IOWait();

// purposefully divide by zero to cause an exception
void CRASH();