#include <stdint.h>
#include <zosdefs.h>
#include <hal/hal.h>
#include "memory.h"
#include "debug.h"
#include <arch/i686/qemu.h>
#include <arch/i686/timer.h>
#include <arch/i686/rtc.h>
#include <arch/i686/pio.h>

#include <boot/bootparams.h>

#include "test.h"

#pragma region 
// libgcc function which calls all global constructors.
// Usually this is done in assembly, but since we skipped that part, we do it here.
extern void _init();

// Test to make sure the global constructors are called.
// Notice how this is never called anywhere in *MY* code.
void __attribute__((constructor)) test_constructor() {
    LogDebug("Constructor", "Global Constructors called.")
}
#pragma endregion

// Kernel Main
void KernelEntry(BootParams* bootParams) {
    // Call all global instructors
    _init();

    LogInfo("Kernel Main", "Boot Device: 0x%x", bootParams->BootDevice);
    LogInfo("Kernel Main", "Memory Region Count: 0x%d", bootParams->Memory.BlockCount);

    LogInfo("Kernel Main", "Beginning kernel initialization...");

    HAL_Initialize(bootParams);

    LogInfo("Kernel Main", "Kernel Initialization Success!");

    rtc_time_t time;
    RTC_GetTime(&time);
    RTC_LogTime("Kernel Main", &time);
    time.hour -= 4;
    RTC_SetTime(&time);
    RTC_GetTime(&time);
    RTC_LogTime("Kernel Main", &time);

    uint8_t* buffer = malloc(512);
    ATA_ReadPIO(bootParams->BootDevice, 0, 1, buffer);
    print_buffer("ATA_ReadPIO returned: ", buffer, 512);

    LogInfo("Kernel Main", "Now we sleep for 10 seconds and then exit!");
    sleep(10000);
    LogInfo("Kernel Main", "We woke up!");
    exit(0);


    HALT
}
