#include "hal.hpp"

#include <core/Debug.hpp>
#include <core/Assert.hpp>
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

    pagingManager.IdentityMapRange(0x00, 16 * 1024 * 1024, PAGE_PRESENT | PAGE_READWRITE);

    extern uint32_t KERNEL_START;
    extern uint32_t KERNEL_END;

    uintptr_t kernel_phys_base = reinterpret_cast<uintptr_t>(&KERNEL_START);
    constexpr uintptr_t kernel_virt_base = 0xC0000000;
    size_t kernel_size = reinterpret_cast<uintptr_t>(&KERNEL_END) - kernel_phys_base;
    pagingManager.MapRange(kernel_phys_base, kernel_virt_base, kernel_size, PAGE_PRESENT | PAGE_READWRITE);

    constexpr uint32_t kernel_stack_virt = 0xC0100000;
    uintptr_t kernel_stack_top = pagingManager.SetupKernelStack(kernel_stack_virt);

    kernel_heap_base = 0xD0000000;
    size_t region_idx = 0;
    best_region = FindBestRegion(&bootparams->Memory, /*out*/ region_idx);

    assert(best_region.Length > kernel_size && "Best memory region is smaller than kernel size!");

    uintptr_t usable_begin = best_region.Begin + kernel_size;
    uintptr_t usable_length = best_region.Length - kernel_size;

    constexpr float FRAME_RATIO = 1.f/10.f;

    size_t frame_allocator_size = (usable_length * FRAME_RATIO);
    frame_allocator_size = (frame_allocator_size / PAGE_SIZE) * PAGE_SIZE;

    size_t heap_size = usable_length - frame_allocator_size;
    assert(heap_size != 0 && "Usable reegion yeilds zero-sized heap!");

    uintptr_t heap_phys_start = usable_begin;
    uintptr_t frame_phys_start = heap_phys_start + heap_size;

    pagingManager.MapRange(heap_phys_start, kernel_heap_base, heap_size, PAGE_PRESENT | PAGE_READWRITE);

    FrameAllocator::Init(frame_phys_start, frame_allocator_size);

    pagingManager.InstallPageDirectory();
    pagingManager.EnablePaging();

    asm volatile("mov %0, %%esp" :: "r"(kernel_stack_top));

    best_region.Begin = heap_phys_start;
    best_region.Length = heap_size;
    
    Debug::Info("HAL", "MMU initialized: heap phys=0x%08X size=%u KB, frame phys=0x%08X size=%u KB",
                heap_phys_start, heap_size / 1024, frame_phys_start, frame_allocator_size / 1024);

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
    //Mem_Init(kernel_heap_base, best_region);
    GDT::LoadDefaults();
    IDT::Load();
    ISR::Init();
    IRQ::Init();
    PIT::Init(1000);
    RTC::Init(true, true);
    ISR::RegisterHandler(14, PageFaultHandler);
    Debug::Info("HAL", "Initialization finished successfully.");
    Mem_Init(kernel_heap_base, best_region);

    return pagingManager;
}
