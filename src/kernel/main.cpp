#include <stdint.h>
#include <zosdefs.h>
#include <hal/hal.hpp>
#include <arch/i686/qemu.h>
#include <arch/i686/timer.h>
#include <arch/i686/rtc.hpp>
#include <arch/i686/pio.h>

#include <boot/bootparams.h>

#include "test.hpp"

#include <arch/i686/pci.hpp>

#include <core/arch/i686/E9Device.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/dev/TextDevice.hpp>
#include <core/Debug.hpp>

#include <core/std/vector.hpp>

#pragma region 
// libgcc function which calls all global constructors.
// Usually this is done in assembly, but since we skipped that part, we do it here.
extern "C" void _init();

// Test to make sure the global constructors are called.
// Notice how this is never called anywhere in *MY* code.
void __attribute__((constructor)) test_constructor() {
    Debug::Debug("Constructor", "Global Constructors called.");
}
#pragma endregion

arch::i686::E9Device g_E9Device{};
arch::i686::VGATextDevice g_VGADevice{};

arch::i686::VGATextDevice* GetGlobalVGADevice() {
    return &g_VGADevice;
}

arch::i686::E9Device* GetGlobalE9Device() {
    return &g_E9Device;
}

// Kernel Main
extern "C" void KernelEntry(BootParams* bootParams) {
    // Call all global instructors
    _init();
    HAL_Initialize(bootParams, &g_VGADevice);

    TextDevice e9_debug{ &g_E9Device };
    TextDevice vga_text{ &g_VGADevice };
    Debug::AddOutputDevice(&vga_text, Debug::DebugLevel::Debug, false);
    Debug::AddOutputDevice(&e9_debug, Debug::DebugLevel::Debug, true);

    Debug::Info("Kernel Main", "Kernel Initialization Success!");

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
    Debug::Info("Kernel Main", "RTL8139 PCI Device: Vendor: %x | Device: %x", dev.vendor_id, dev.device_id);

    Debug::Info("Kernel Main", "Now we sleep for 10 seconds and then exit!");
    sleep(10000);
    Debug::Info("Kernel Main", "We woke up!");


    exit(0);

    

    HALT
}
