#include <stdint.h>
#include <zosdefs.h>
#include <hal/hal.h>
#include "memory.h"
#include "debug.h"
#include <arch/i686/qemu.h>

#include <boot/bootparams.h>

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

    HAL_Initialize();

    LogInfo("Kernel Main", "Kernel Initialization Success!");

    LogDebug("Debug", "This is a debug message!");
    LogInfo("Info", "This is an info message!");
    LogWarn("Warn", "This is a warning message!");
    LogError("Error", "This is an error message!");
    LogCritical("Critical", "This is a critical failure message!!");

    mem_init(&bootParams->Memory);
    void* a = malloc(100);
    void* b = malloc(200);
    LogInfo("Stage2 Main", "Mem Test &a = 0x%p &b = 0x%p", a, b);
    free(a);
    void* c = malloc(50);
    LogInfo("Stage2 Main", "Mem Test &c = 0x%p", c);
    free(b);
    free(c);
    a = malloc(0x10000);
    b = malloc(48);
    LogInfo("Stage2 Main", "Mem Test new &a = 0x%p new &b = 0x%p", a, b);


    exit(0);
    HALT
}
