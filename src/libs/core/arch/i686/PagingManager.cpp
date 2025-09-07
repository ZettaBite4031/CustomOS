#include "PagingManager.hpp"

#include <core/Assert.hpp>

extern "C" {
    char KERNEL_END; // Symbol defined in linker script
};

namespace {
    class PhysicalMemoryManager {
    public:
        static void Init(const MemoryInfo& memory) {
        // Find the best region (largest usable memory after kernel)
        uintptr_t best_start = 0;
        uintptr_t best_len = 0;


        uintptr_t kernel_end = (uintptr_t)&KERNEL_END;

        for (int i = 0; i < memory.BlockCount; ++i) {
            const MemoryRegion& region = memory.Regions[i];
            if (region.Type != 1) continue; // Type 1 = usable

            uintptr_t region_start = (uintptr_t)region.Begin;
            uintptr_t region_end = region_start + (uintptr_t)region.Length;

            if (region_end <= kernel_end) continue;

            // Adjust to not overlap the kernel
            uintptr_t usable_start = (region_start < kernel_end) ? kernel_end : region_start;
            uintptr_t usable_len = region_end - usable_start;

            if (usable_len > best_len) {
                best_start = usable_start;
                best_len = usable_len;
            }
        }

        // Align start to 4KB
        m_NextFreePage = (best_start + 0xFFF) & ~0xFFF;
        m_Limit = best_start + best_len;
    }

    static void* AllocatePage() {
        if (m_NextFreePage + 0x1000 > m_Limit) {
            // Out of memory (you could trigger panic here)
            return nullptr;
        }

        void* page = (void*)m_NextFreePage;
        m_NextFreePage += 0x1000;
        return page;
    }

    private:
        static uintptr_t m_NextFreePage;
        static uintptr_t m_Limit;
    };

    uintptr_t PhysicalMemoryManager::m_NextFreePage = 0;
    uintptr_t PhysicalMemoryManager::m_Limit = 0;
}

PagingManager::PagingManager(const MemoryInfo& mem_info) {
    PhysicalMemoryManager::Init(mem_info);
    PageDirectory = (uint32_t*)AllocatePageAligned();
    Memory::Set(PageDirectory, 0, PAGE_SIZE);
}

void PagingManager::MapRange(uintptr_t phys_start, uintptr_t virt_start, size_t size, uint32_t flags) {
    size_t pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t i = 0; i < pages; i++) {
        uintptr_t phys_addr = phys_start + i * PAGE_SIZE;
        uintptr_t virt_addr = virt_start + i * PAGE_SIZE;

        MapPage(phys_addr, virt_addr, flags);
    }
}

void PagingManager::MapPage(uintptr_t phys_addr, uintptr_t virt_addr, uint32_t flags) {
    uint32_t pd_index = (virt_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    uint32_t* page_table = GetPageTable(pd_index, true);
    page_table[pt_index] = (phys_addr & 0xFFFFF000) | (flags & 0xFFF);

    invlpg((void*)virt_addr);
}

void PagingManager::IdentityMapRange(uintptr_t start, size_t size, uint32_t flags) {
    MapRange(start, start, size, flags);
}

uintptr_t PagingManager::PhysToVirt(uintptr_t phys_addr) const {
    return phys_addr + KernelVirtualOffset;
}

uintptr_t PagingManager::VirtToPhys(uintptr_t virt_addr) {
    uint32_t pd_index = (virt_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virt_addr >> 12) & 0x3FF;

    uint32_t pde = PDE(pd_index);
    if (!(pde & PAGE_PRESENT)) return 0; // not mapped;

    uint32_t* page_table = (uint32_t*)(pde & 0xFFFFF000);
    uint32_t pte = page_table[pt_index];
    if (!(pte & PAGE_PRESENT)) return 0;

    return (pte & 0xFFFFF000) | (virt_addr & 0xFFF);
}

void PagingManager::InstallPageDirectory() {
    load_cr3((uint32_t)PageDirectory);
}

void PagingManager::EnablePaging() {
    enable_paging();
}

uintptr_t PagingManager::SetupKernelStack(uintptr_t virt_stack_base, size_t stack_pages, uint32_t flags) {
    for (size_t i = 0; i < stack_pages; i++) {
        void* phys_page = AllocatePageAligned();
        uintptr_t virt_addr = virt_stack_base + i * PAGE_SIZE;
        MapRange((uintptr_t)phys_page, virt_addr, PAGE_SIZE, flags);
    }

    return virt_stack_base + stack_pages * PAGE_SIZE;
}

uint32_t* PagingManager::GetPageTable(uint32_t pd_index, bool create_if_missing) {
    uint32_t pde = PDE(pd_index);
    if (!(pde & PAGE_PRESENT)) {
        if (!create_if_missing) return nullptr;

        void* page_table = AllocatePageAligned();
        Memory::Set(page_table, 0, PAGE_SIZE);
        PDE(pd_index) = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_READWRITE;
        return (uint32_t*)page_table;
    }
    return (uint32_t*)(pde & 0xFFFFF000);
}

uint32_t& PagingManager::PTE(uint32_t pd_index, uint32_t pt_index) {
    uint32_t* page_table = GetPageTable(pd_index, true);
    return page_table[pt_index];
}


void* PagingManager::AllocatePageAligned() {
    void* page = PhysicalMemoryManager::AllocatePage();
    Memory::Set(page, 0, PAGE_SIZE);
    return page;
}