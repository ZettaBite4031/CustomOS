#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>

#include <stdio.h>
#include <debug.h>

void HAL_Initialize() {
    VGA_cls();
    printf("=-=-=-=-= ZOS KERNEL LOADING =-=-=-=-=\n\n");
    printf("HAL: Hardware Abstraction Layer beginning initialization...\n");
    LogInfo("HAL", "Beginning Initialization...");
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    printf("HAL: Initialization finished successfully.\n");
    LogInfo("HAL", "Initialization Success!");
}
