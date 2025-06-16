#include "hal.hpp"

#include <core/Debug.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/arch/i686/E9Device.hpp>
#include <core/cpp/Memory.hpp>

#include <core/arch/i686/GDT.hpp>
#include <core/arch/i686/IDT.hpp>
#include <core/arch/i686/ISR.hpp>
#include <core/arch/i686/IRQ.hpp>
#include <core/arch/i686/Timer.hpp>
#include <core/arch/i686/RTC.hpp>

arch::i686::E9Device g_E9Device{};
TextDevice e9_debug{ &g_E9Device };

arch::i686::VGATextDevice g_VGADevice{};
TextDevice vga_text{ &g_VGADevice };

arch::i686::VGATextDevice* GetGlobalVGADevice() {
    return &g_VGADevice;
}

arch::i686::E9Device* GetGlobalE9Device() {
    return &g_E9Device;
}

void HAL_Initialize(BootParams* bootparams) {
    Mem_Init(&bootparams->Memory);

    Debug::AddOutputDevice(&vga_text, Debug::DebugLevel::Info, false);
    Debug::AddOutputDevice(&e9_debug, Debug::DebugLevel::Debug, true);

    g_VGADevice.ClearScreen();
    Debug::Info("ZOS", "=-=-=-=-= ZOS KERNEL LOADING =-=-=-=-=");
    Debug::Info("HAL", "Hardware Abstraction Layer beginning initialization...");
    GDT::LoadDefaults();
    IDT::Load();
    ISR::Init();
    IRQ::Init();
    PIT::Init(1000);
    RTC::Init(true, true);
    Debug::Info("HAL", "Initialization finished successfully.");
}
