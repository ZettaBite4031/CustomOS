#include "hal.h"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.h>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/vga_text.h>
#include <arch/i686/timer.h>
#include <arch/i686/rtc.h>

#include <stdio.h>
#include <debug.h>
#include "memory.h"

void HAL_Initialize(BootParams* bootparams) {
    VGA_cls();
    LogInfo("ZOS", "=-=-=-=-= ZOS KERNEL LOADING =-=-=-=-=");
    LogInfo("HAL", "Hardware Abstraction Layer beginning initialization...");
    LogInfo("HAL", "Beginning Initialization...");
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    Mem_Init(&bootparams->Memory);
    PIT_Init(1000);
    RTC_Init(true, true);
    LogInfo("HAL", "Initialization finished successfully.");
    LogInfo("HAL", "Initialization Success!");
}
