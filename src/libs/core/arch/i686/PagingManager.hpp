#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/cpp/Memory.hpp>
#include <boot/bootparams.h>


constexpr uint32_t PAGE_PRESENT     = (1 << 0);
constexpr uint32_t PAGE_READWRITE   = (1 << 1);
constexpr uint32_t PAGE_USER        = (1 << 2);
constexpr uint32_t PAGE_WRITETHROUGH = (1 << 3);
constexpr uint32_t PAGE_CACHEDISABLED = (1 << 4);

constexpr uint32_t PAGE_MMIO = PAGE_PRESENT | PAGE_READWRITE | PAGE_CACHEDISABLED;

constexpr size_t PAGE_SIZE = 4096;
constexpr size_t PAGE_ENTRIES = 1024;

extern "C" void load_cr3(uint32_t);
extern "C" void enable_paging();
extern "C" void invlpg(void*);

class PagingManager {
public:
    PagingManager(const MemoryInfo& mem_info);

    void MapRange(uintptr_t phys_start, uintptr_t virt_start, size_t size, uint32_t flags);
    void IdentityMapRange(uintptr_t start, size_t size, uint32_t flags);
    void MapPage(uintptr_t phys_addr, uintptr_t virt_addr, uint32_t flags);

    uintptr_t PhysToVirt(uintptr_t phys_addr) const;
    uintptr_t VirtToPhys(uintptr_t virt_addr);

    void InstallPageDirectory();
    void EnablePaging();

    uintptr_t SetupKernelStack(uintptr_t virt_stack_base, size_t stack_pages = 1, uint32_t flags = PAGE_PRESENT | PAGE_READWRITE);

private:
    uint32_t* PageDirectory;

    void* AllocatePageAligned();
    uint32_t* GetPageTable(uint32_t pd_index, bool create_if_missing = true);

    uint32_t& PDE(uint32_t pd_index) { return PageDirectory[pd_index]; }
    uint32_t& PTE(uint32_t pd_index, uint32_t pt_index);

    uintptr_t KernelVirtualOffset = 0xC0000000;
};
