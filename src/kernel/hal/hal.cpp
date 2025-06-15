#include "hal.hpp"
#include <arch/i686/gdt.h>
#include <arch/i686/idt.hpp>
#include <arch/i686/isr.h>
#include <arch/i686/irq.h>
#include <arch/i686/timer.h>
#include <arch/i686/rtc.hpp>

#include <stdio.h>
#include <core/Debug.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/cpp/Memory.hpp>

void HAL_Initialize(BootParams* bootparams, arch::i686::VGATextDevice* vga) {
    Mem_Init(&bootparams->Memory);
    vga->ClearScreen();
    Debug::Info("ZOS", "=-=-=-=-= ZOS KERNEL LOADING =-=-=-=-=");
    Debug::Info("HAL", "Hardware Abstraction Layer beginning initialization...");
    Debug::Info("HAL", "Beginning Initialization...");
    i686_GDT_Initialize();
    i686_IDT_Initialize();
    i686_ISR_Initialize();
    i686_IRQ_Initialize();
    PIT_Init(1000);
    RTC::Init(true, true);
    Debug::Info("HAL", "Initialization finished successfully.");
    Debug::Info("HAL", "Initialization Success!");
}
