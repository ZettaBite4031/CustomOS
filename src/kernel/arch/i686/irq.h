#pragma once
#include "isr.h"

typedef void(*IRQHandler)(Registers*);

void i686_IRQ_Initialize();
void i686_IRQ_RegisterHandler(int irq, IRQHandler handler);
