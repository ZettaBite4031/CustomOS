#pragma once
#include <stdint.h>
#include <zosdefs.h>

typedef struct {
    uint32_t ds;
    uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;
    uint32_t interrupt, error;
    uint32_t eip, cs, eflags, esp, ss;
} PACKED Registers;

typedef void(*ISRHandler)(Registers* regs);

void i686_ISR_Initialize();
void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler);
