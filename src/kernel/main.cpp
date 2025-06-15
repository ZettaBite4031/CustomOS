#include <stdint.h>
#include <zosdefs.h>
#include <hal/hal.h>
#include "memory.h"
#include "debug.h"
#include <arch/i686/qemu.h>
#include <arch/i686/timer.h>
#include <arch/i686/rtc.hpp>
#include <arch/i686/pio.h>

#include <boot/bootparams.h>

#include "test.h"

#include <arch/i686/pci.hpp>

#include <core/std/vector.hpp>

#pragma region 
// libgcc function which calls all global constructors.
// Usually this is done in assembly, but since we skipped that part, we do it here.
extern "C" void _init();

// Test to make sure the global constructors are called.
// Notice how this is never called anywhere in *MY* code.
void __attribute__((constructor)) test_constructor() {
    LogDebug("Constructor", "Global Constructors called.")
}
#pragma endregion

// Kernel Main
extern "C" void KernelEntry(BootParams* bootParams) {
    // Call all global instructors
    _init();


    LogInfo("Kernel Main", "Boot Device: 0x%x", bootParams->BootDevice);
    LogInfo("Kernel Main", "Memory Region Count: 0x%d", bootParams->Memory.BlockCount);

    LogInfo("Kernel Main", "Beginning kernel initialization...");

    HAL_Initialize(bootParams);

    LogInfo("Kernel Main", "Kernel Initialization Success!");

    RTC::RTCTime time;
    RTC::GetTime(time);
    RTC::LogTime("Kernel Main", time);
    time.hour -= 4;
    RTC::SetTime(time);
    RTC::GetTime(time);
    RTC::LogTime("Kernel Main", time);

    std::vector<uint8_t> test_buffer(512);
    ATA_ReadPIO(bootParams->BootDevice, 0, 1, test_buffer.data());
    print_buffer("ATA_ReadPIO w/ Vector returned: ", test_buffer.data(), test_buffer.size());

    PCI_Enumerate();

    pci_dev_t dev;
    PCI_GetRTL8139(dev);
    LogInfo("Kernel Main", "RTL8139 PCI Device: Vendor: %x | Device: %x", dev.vendor_id, dev.device_id);

    LogInfo("Kernel Main", "Now we sleep for 10 seconds and then exit!");
    sleep(10000);
    LogInfo("Kernel Main", "We woke up!");

    exit(0);

    

    HALT
}
