#include <stdint.h>
#include <zosdefs.h>
#include <hal/hal.h>
#include "memory.h"
#include "debug.h"

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
void KernelEntry() {
    // Call all global instructors
    _init();

    LogInfo("Kernel Main", "Beginning kernel initialization...");

    HAL_Initialize();

    LogInfo("Kernel Main", "Kernel Initialization Success!");

    LogDebug("Debug", "This is a debug message!");
    LogInfo("Info", "This is an info message!");
    LogWarn("Warn", "This is a warning message!");
    LogError("Error", "This is an error message!");
    LogCritical("Critical", "This is a critical failure message!!");

    HALT
}
