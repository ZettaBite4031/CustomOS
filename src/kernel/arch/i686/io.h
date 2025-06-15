#pragma once
#include <stdint.h>
#include <zosdefs.h>


extern "C" void EXTERN i686_OutB(uint16_t port, uint8_t value);
extern "C" uint8_t EXTERN i686_InB(uint16_t port);
extern "C" void EXTERN i686_OutW(uint16_t port, uint16_t value);
extern "C" uint16_t EXTERN i686_InW(uint16_t port);
extern "C" void EXTERN i686_OutL(uint16_t port, uint32_t value);
extern "C" uint32_t EXTERN i686_InL(uint16_t port);

extern "C" void EXTERN i686_EnableInterrupts();
extern "C" void EXTERN i686_DisableInterrupts();

// halt the device during a panic state.
extern "C" void EXTERN i686_PANIC();

void i686_IOWait();

// purposefully divide by zero to cause an exception
void CRASH();