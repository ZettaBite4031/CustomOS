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
    size_t region_idx = 0;
    best_region = FindBestRegion(&bootparams->Memory, region_idx);

    // Reserve space for FrameAllocator bitmap and heap allocator linked list
    constexpr size_t frame_allocator_bitmap_size = 65536;  // FrameAllocator's static bitmap size
    constexpr size_t heap_linked_list_reserve = 65536;     // Reserve space for heap's linked list
    constexpr size_t total_reserved = frame_allocator_bitmap_size + heap_linked_list_reserve;

    // Calculate available memory after kernel and reservations
    size_t available_memory = best_region.Length - kernel_size;
    if (available_memory <= total_reserved) {
        Debug::Critical("HAL", "Insufficient memory for allocators! Available: %dMB, Need: %dMB",
                       available_memory / 1024 / 1024, total_reserved / 1024 / 1024);
        // Fallback to minimal allocation
        available_memory = total_reserved;
    }

    // Place FrameAllocator bitmap at the beginning of available memory
    uintptr_t frame_allocator_bitmap_start = best_region.Begin + kernel_size;
    // Note: FrameAllocator uses a static bitmap, so we don't need to allocate space for it here

    // Place heap linked list after FrameAllocator bitmap
    uintptr_t heap_linked_list_start = frame_allocator_bitmap_start + frame_allocator_bitmap_size;

    // Calculate actual heap size (remaining memory after reservations)
    size_t actual_heap_size = available_memory - frame_allocator_bitmap_size;

    // Map heap region
    pagingManager.MapRange(heap_linked_list_start, kernel_heap_base, actual_heap_size, PAGE_PRESENT | PAGE_READWRITE);

    // Find a separate region for FrameAllocator (not overlapping with heap)
    size_t frame_allocator_size = 2 * 1024 * 1024;  // 2MB for FrameAllocator
    uintptr_t frame_allocator_start = 0;

    // Look for a memory region that's not the best region (to avoid conflicts)
    for (size_t i = 0; i < bootparams->Memory.BlockCount; ++i) {
        MemoryRegion region = bootparams->Memory.Regions[i];
        if (region.Type != 1) continue; // Only usable RAM

        // Skip if this is the same region as our heap
        if (region_idx == i) continue;

        // Check if this region has enough space for FrameAllocator
        if (region.Length >= frame_allocator_size) {
            frame_allocator_start = region.Begin;
            break;
        }
    }

    if (frame_allocator_start == 0) {
        Debug::Critical("HAL", "No suitable memory region found for FrameAllocator!");
        // Fallback: use the end of the best region (this might cause issues)
        frame_allocator_start = heap_linked_list_start + actual_heap_size;
        frame_allocator_size = 1 * 1024 * 1024;  // Reduce size for fallback
    }

    FrameAllocator::Init(frame_allocator_start, frame_allocator_size);

    // Adjust best_region to reflect the actual heap size for Mem_Init
    best_region.Length = actual_heap_size;
    // Initialize heap allocator with the physical address of the linked list area
    Mem_Init(heap_linked_list_start, best_region);

    Debug::Info("HAL", "Memory Layout:");
    Debug::Info("HAL", "  FrameAllocator bitmap: 0x%08X", frame_allocator_bitmap_start);
    Debug::Info("HAL", "  Heap linked list: 0x%08X", heap_linked_list_start);
    Debug::Info("HAL", "  Heap virtual: 0x%08X", kernel_heap_base);
    Debug::Info("HAL", "  FrameAllocator phys: 0x%08X", frame_allocator_start);
    Debug::Info("HAL", "  Total reserved: %dMB", total_reserved / 1024 / 1024);

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
    //Mem_Init(kernel_heap_base, best_region);
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
