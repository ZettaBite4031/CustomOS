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

#include <core/cpp/Memory.hpp>
#include <core/dev/MBR.hpp>

#include <core/fs/FATFileSystem.hpp>
#include <core/dev/RangeBlockDevice.hpp>
#include <core/arch/i686/Disk.hpp>

#include <core/cpp/String.hpp>

#pragma region 
// libgcc function which calls all global constructors.
// Usually this is done in assembly, but since we skipped that part, we do it here.
extern "C" void _init();

// Test to make sure the global constructors are called.
// Notice how this is never called anywhere in *MY* code.
void __attribute__((constructor)) test_constructor() {
    Debug::Debug("Constructor", "Global Constructors called.");
}

arch::i686::E9Device g_E9Device{};
arch::i686::VGATextDevice g_VGADevice{};

arch::i686::VGATextDevice* GetGlobalVGADevice() {
    return &g_VGADevice;
}

arch::i686::E9Device* GetGlobalE9Device() {
    return &g_E9Device;
}
#pragma endregion

void EoH(int exit_code) {
    exit(exit_code);
    HALT;
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
    Debug::Info("Kernel Main", "RTL8139 PCI Device: Vendor: %X | Device: %X", dev.vendor_id, dev.device_id);

    IOAllocator KernelIOAllocator{};
    IORange disk_pio_range = KernelIOAllocator.RequestIORange(0x1F0, 8, false);
    Disk disk{ bootParams->BootDevice, &disk_pio_range, true };
    if (!disk.Initialize()) {
        Debug::Critical("Kernel Main", "Disk initialization failed!");
        EoH(1);
    }
    
    BlockDevice* partition;
    RangeBlockDevice partitionRange;
    if (bootParams->BootDevice < 0x80) {
        partition = &disk;
    } else {
        MBR_entry* entry = ToLinear<MBR_entry*>(reinterpret_cast<uint32_t>(bootParams->PartitionLocation));
        partitionRange.Initialize(&disk, entry->LBA_Start * Disk::BytesPerSector, entry->SectorCount * Disk::BytesPerSector);
        partition = &partitionRange;
    }

    FATFileSystem fatfs;
    if (!fatfs.Initialize(partition)) {
        Debug::Critical("Kernel Main", "Failed to initialize FATFS");
        EoH(1);
    }

    const char* file_path = "/folder/demo.txt";
    File* test = fatfs.Open(file_path, FileOpenMode::Read);
    if (!test) {
        Debug::Critical("Kernel Main", "Failed to open %s", file_path);
        EoH(1);
    }
    std::vector<uint8_t> text_data(test->Size());
    test->Read(text_data.data(), text_data.size());
    text_data.emplace_back('\0');
    Debug::Info("Kernel Main", "%s contents:\n%s", file_path, text_data.data());
    const char* text = "I'm sooooo bored~";
    test->Write(reinterpret_cast<const uint8_t*>(text), strlen(text));
    test->Seek(0, SeekPos::Set);
    text_data.clear();
    text_data.resize(test->Size());
    test->Read(text_data.data(), text_data.size());
    text_data[text_data.size()] = '\0';
    Debug::Info("Kernel Main", "%s contents:\n%s", file_path, text_data.data());

    Debug::Info("Kernel Main", "Now we sleep for 2.5 seconds and then exit!");
    sleep(2500);
    Debug::Info("Kernel Main", "We woke up!");


    EoH(0);
}
