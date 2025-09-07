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
#include <core/arch/i686/FrameAllocator.hpp>
#include <core/arch/i686/PagingManager.hpp>

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

void PageFaultHandler(ISR::Registers* regs) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));
    Debug::Critical("PageFault", "Page fault at addr '0x%X'", faulting_address);
    arch::i686::PANIC();
}

PagingManager InitializeMMU(BootParams* bootparams, uintptr_t& kernel_heap_base, MemoryRegion& best_region) {
    PagingManager pagingManager(bootparams->Memory);

    // Identity map the first 16MB of RAM
    pagingManager.IdentityMapRange(0x00000000, 16 * 1024 * 1024, PAGE_PRESENT | PAGE_READWRITE);

    // Map kernel
    extern uint32_t KERNEL_START;
    extern uint32_t KERNEL_END;
    uintptr_t kernel_phys_base = reinterpret_cast<uintptr_t>(&KERNEL_START);
    constexpr uintptr_t kernel_virt_base = 0xC0000000;
    size_t kernel_size = reinterpret_cast<size_t>(&KERNEL_END) - reinterpret_cast<size_t>(&KERNEL_START);
    pagingManager.MapRange(kernel_phys_base, kernel_virt_base, kernel_size, PAGE_PRESENT | PAGE_READWRITE);

    // Map kernel stack
    constexpr uint32_t kernel_stack_virt = 0xC0100000;
    uintptr_t kernel_stack_top = pagingManager.SetupKernelStack(kernel_stack_virt);

    // Find and map best memory region
    kernel_heap_base = 0xD0000000;
    best_region = FindBestRegion(&bootparams->Memory);
    pagingManager.MapRange(best_region.Begin + kernel_size, kernel_heap_base, best_region.Length - kernel_size, PAGE_PRESENT | PAGE_READWRITE);

    size_t frame_allocator_size = 2 * 1024 * 1024;
    FrameAllocator::Init(kernel_phys_base + kernel_size + best_region.Length, frame_allocator_size);

    // Enable the MMU
    pagingManager.InstallPageDirectory();
    pagingManager.EnablePaging();

    // Set kernel stack
    asm volatile("mov %0, %%esp" :: "r"(kernel_stack_top));

    return pagingManager;
}

PagingManager HAL_Initialize(BootParams* bootparams) {
    uintptr_t kernel_heap_base = 0;
    MemoryRegion best_region{ 0 };
    PagingManager pagingManager = InitializeMMU(bootparams, kernel_heap_base, best_region);
    Debug::Init();
    Debug::AddOutputDevice(&vga_text, Debug::DebugLevel::Info, false);
    Debug::AddOutputDevice(&e9_debug, Debug::DebugLevel::Debug, true);
    
    g_VGADevice.ClearScreen();
    Debug::Info("ZOS", "=-=-=-=-= ZOS KERNEL LOADING =-=-=-=-=");
    Debug::Info("HAL", "Hardware Abstraction Layer beginning initialization...");
    Mem_Init(kernel_heap_base, best_region);
    GDT::LoadDefaults();
    IDT::Load();
    ISR::Init();
    IRQ::Init();
    PIT::Init(1000);
    RTC::Init(true, true);
    ISR::RegisterHandler(14, PageFaultHandler);
    Debug::Info("HAL", "Initialization finished successfully.");
    return pagingManager;
}
