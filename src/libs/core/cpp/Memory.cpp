#include "Memory.hpp"
#include <core/Debug.hpp>

uint32_t get_alignment(void* ptr) {
    uintptr_t a = (uintptr_t)ptr;
    // take the lowest set bit of a, up to 64:
    uint32_t align = a & -a;
    return (align > 64 ? 64 : align);
}


struct Block {
    uint32_t size;
    bool free;
    Block* next;
    Block* prev;
};

extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;

static Block* g_FreeList = nullptr;

MemoryRegion FindBestRegion(MemoryInfo* info) {
    uintptr_t kernel_start = (uintptr_t)&KERNEL_START;
    uintptr_t kernel_end   = (uintptr_t)&KERNEL_END;

    MemoryRegion best = {0};

    for (size_t i = 0; i < info->BlockCount; ++i) {
        MemoryRegion region = info->Regions[i];

        if (region.Type != 1) continue; // Only usable RAM

        uintptr_t start = region.Begin;
        uintptr_t end   = region.Begin + region.Length;

        // Skip region entirely if it ends before or starts after the kernel
        if (end <= kernel_start || start >= kernel_end) {
            // No overlap with kernel at all, fully usable
            if (region.Length > best.Length)
                best = region;
        }
        // If region overlaps kernel partially or fully
        else {
            // Adjust the region to start after the kernel ends
            if (start < kernel_end && end > kernel_end) {
                uintptr_t adjusted_start = kernel_end;
                uintptr_t adjusted_length = end - kernel_end;
                if (adjusted_length > best.Length) {
                    best.Begin = adjusted_start;
                    best.Length = adjusted_length;
                    best.Type = region.Type;
                }
            }
            // Region starts inside kernel or exactly where kernel starts (fully unusable portion)
            // e.g., region.Begin == kernel_start and end <= kernel_end
            // => skip it because no usable memory left
        }
    }

    return best;
}


bool Mem_Init(uintptr_t heap_base, const MemoryRegion& best_region) {
    g_FreeList = (Block*)(heap_base);
    memset(g_FreeList, 0x00, sizeof(Block));
    g_FreeList->size = best_region.Length - sizeof(Block);
    g_FreeList->free = true;
    g_FreeList->next = nullptr;
    g_FreeList->prev = nullptr;

    Debug::Info("MemInit", "Memory initialized! Region [%08X - %08X] (%dMB)", best_region.Begin, best_region.Begin + best_region.Length, best_region.Length / 1024 / 1024);
    return true;
}

void* malloc_aligned(uint32_t size, uint32_t alignment) {
    Block* current = g_FreeList;

    while (current) {
        if (!current->free) {
            current = current->next;
            continue;
        }

        uintptr_t raw_block = (uintptr_t)current;
        uintptr_t user_data = ALIGN_UP(raw_block + sizeof(Block) + sizeof(void*), alignment);
        uintptr_t block_end = user_data + size;
        uintptr_t total_size = block_end - raw_block;

        if (current->size >= total_size) {
            if (current->size >= total_size + sizeof(Block) + 8) {
                Block* split = (Block*)(raw_block + total_size);
                split->size = current->size - total_size;
                split->free = true;
                split->next = current->next;
                split->prev = current;

                if (current->next) current->next->prev = split;
                current->next = split;
                current->size = total_size - sizeof(Block);
            }

            current->free = false;
            uintptr_t backref = user_data - sizeof(void*);
            *((Block**)backref) = current;
            return (void*)user_data;
        }
        current = current->next;
    }

    return nullptr;
}

void free(void* ptr) {
    if (!ptr) return;

    uintptr_t backref = (uintptr_t)ptr - sizeof(void*);
    Block* block = *((Block**)backref);
    block->free = true;

    if (block->next && block->next > block && block->next->free) {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(Block) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void* realloc(void* ptr, uint32_t new_size) {
    if (!ptr) return malloc_aligned(new_size, 8);
    
    if (new_size == 0) {
        free(ptr);
        return nullptr;
    }

    uintptr_t backref = (uintptr_t)ptr - sizeof(void*);
    Block* block = *((Block**)backref);
    if (block->size >= new_size) return ptr;

    if (block->next && block->next->free) {
        uint32_t combined_size = block->size + sizeof(Block) + block->next->size;
        if (combined_size >= new_size) {
            Block* next = block->next;
            block->size = combined_size;
            block->next = next->next;
            if (next->next) next->next->prev = block;
            return ptr;
        }
    }

    void* new_ptr = malloc_aligned(new_size, 8);
    if (!new_ptr) return nullptr;

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    return new_ptr;
}

void* malloc(uint32_t size) {
    return malloc_aligned(size, 8);
}

void* calloc(uint32_t size) {
    void* ptr = malloc(size);
    if (!ptr) return nullptr;
    memset(ptr, 0x00, size);
    return ptr;
}

void* recalloc(void* ptr, uint32_t new_size) {
    void* new_ptr = realloc(ptr, new_size);
    if (!new_ptr) return nullptr;
    memset(new_ptr, 0x00, new_size);
    return ptr;
}

void* operator new(size_t size) {
    return malloc(static_cast<uint32_t>(size));
}

void* operator new[](size_t size) {
    return malloc(static_cast<uint32_t>(size));
}

void operator delete(void* ptr) {
    free(ptr);
}

void operator delete[](void* ptr) {
    free(ptr);
}

void* operator new(size_t size, size_t align) {
    return malloc_aligned(static_cast<uint32_t>(size), static_cast<uint32_t>(align));
}

void operator delete(void* ptr, size_t align) {
    free(ptr);
}

void* operator new[](size_t size, size_t align) {
    return malloc_aligned(static_cast<uint32_t>(size), static_cast<uint32_t>(align));
}

void operator delete[](void* ptr, size_t align) {
    free(ptr);
}

void operator delete(void*, void* ptr) {
    // Do nothing
}

void DumpHeap() {
    Debug::Info("Allocator", "================ HEAP DUMP ================");
    Block* current = g_FreeList;
    int index = 0;
    size_t total_free = 0, total_used = 0;

    while (current) {
        const char* status = current->free ? "free" : "used";
        Debug::Info("Allocator", "#%02d - Addr: %p | Size: %08zu | Status: %s", index++, current, current->size, status);
        if (current->free) total_free += current->size;
        else total_used += current->size;
        current = current->next;
    }

    Debug::Info("Allocator", "Total Used: %zu bytes (%zuMB)", total_used, total_used / 1024 / 1024);
    Debug::Info("Allocator", "Total Free: %zu bytes (%zuMB)", total_free, total_free / 1024 / 1024);
    Debug::Info("Allocator", "================ HEAP DUMP ================");
}