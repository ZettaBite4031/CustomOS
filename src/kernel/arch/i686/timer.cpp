#include "timer.h"
#include "irq.h"
#include "pic.h"
#include "io.h"
#include "i8259.h"

#include <stddef.h>

volatile uint32_t pit_ticks = 0;
const PIC_Driver* g_i8259 = NULL;
uint32_t g_PIT_HZ = 0;


void PIT_Handler(Registers* regs) {
    pit_ticks++;
    g_i8259->SendEOI(0);
}

void PIT_Init(uint32_t freq_hz) {
    g_i8259 = i8259_GetDriver();
    g_PIT_HZ = freq_hz;
    
    uint32_t divisor = 1193182 / freq_hz;

    i686_OutB(0x43, 0x36);
    i686_OutB(0x40, (uint8_t)(divisor & 0xFF));
    i686_OutB(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    i686_IRQ_RegisterHandler(0, PIT_Handler);
    g_i8259->Unmask(0x0);
}

void sleep(uint32_t ms) {
    uint32_t start = pit_ticks;
    uint32_t target = (ms * g_PIT_HZ) / 1000;

    // busy-wait but halt the CPU between interrupts
    while ((pit_ticks - start) < target) {
        __asm__ __volatile__("sti; hlt; cli");
        // enable interrupts, halt until next IRQ, then disable again
    }
}