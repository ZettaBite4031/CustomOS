#include "memory.h"
#include "debug.h"

typedef struct block_header {
    uint32_t size;
    bool free;
    struct block_header* next;
    struct block_header* prev;
} block_header_t;

uint32_t get_alignment(void* ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    uint32_t alignment = 1;
    while ((addr & alignment) == 0 && alignment <= 64) alignment <<= 1;
    return alignment >>= 1;
}


extern uint32_t KERNEL_START;
extern uint32_t KERNEL_END;
extern block_header_t FirstFree; 

static block_header_t* free_list = NULL;

MemoryRegion find_suitable_region(MemoryInfo* mem_info) {
    MemoryRegion alloc_block = {0};
    for (int i = 0; i < mem_info->BlockCount; i++) {
        MemoryRegion block = mem_info->Regions[i];
        if (block.Length > alloc_block.Length && block.Type != 0x2 && block.Begin == (uint32_t)&KERNEL_START) alloc_block = block;
    }
    return alloc_block;
}

bool mem_init(MemoryInfo* mem_info) {
    MemoryRegion region = find_suitable_region(mem_info);
    if (region.Length == 0) {
        LogCritical("MemInit", "Could not find suitable region for the allocator!");
        return false;
    }
    LogInfo("MemInit", "Allocator region:\n  Base: 0x%llx\n  Length: 0x%llx (%dMB)\n  Type: 0x%x\n  ACPI: 0x%x", region.Begin, region.Length, region.Length / 1024 / 1024, region.Type, region.ACPI);

    uint32_t kernel_start_addr = (uint32_t)&KERNEL_START;
    uint32_t kernel_end_addr = (uint32_t)&KERNEL_END;
    uint32_t reserved_kernel_length = (kernel_end_addr - kernel_start_addr);
    uint32_t segment_size = region.Length - reserved_kernel_length - sizeof(block_header_t);

    free_list = &FirstFree;
    free_list->size = segment_size;
    free_list->free = true;
    free_list->next = NULL;
    free_list->prev = NULL;

    return true;
}

void* malloc_aligned(uint32_t size, uint32_t alignment) {
    block_header_t* current = free_list;

    while (current) {
        if (current->free) {
            uintptr_t block_addr = (uintptr_t)current;
            uintptr_t user_start = ALIGN_UP(block_addr + sizeof(block_header_t) + sizeof(void*), alignment);
            uintptr_t total_size = (user_start + size) - block_addr;

            if (current->size >= total_size) {
                // If enough room to split
                if (current->size > total_size + sizeof(block_header_t)) {
                    block_header_t* new_block = (block_header_t*)(block_addr + total_size);
                    new_block->size = current->size - total_size - sizeof(block_header_t);
                    new_block->free = true;
                    new_block->prev = current;
                    
                    if (current->next) current->next->prev = new_block;
                    current->next = new_block;
                    current->size = total_size - sizeof(block_header_t);
                }

                current->free = false;

                // Store backref to header before aligned region
                uintptr_t backref_addr = user_start - sizeof(void*);
                *((block_header_t**)backref_addr) = current;

                return (void*)user_start;
            }
        }
        current = current->next;
    }

    return NULL; // no suitable block found
}

void* malloc(uint32_t size) {
    return malloc_aligned(size, get_alignment(free_list));
}

void* calloc(uint32_t size) {
    void* ptr = malloc(size);
    memset(ptr, 0, size);
    return ptr;
}

void free(void* ptr) {
    free_aligned(ptr);
}

void free_aligned(void* ptr) {
    if (!ptr) return;

    uintptr_t user_ptr = (uintptr_t)ptr;
    block_header_t* block = *((block_header_t**)(user_ptr - sizeof(void*)));
    block->free = true;

    // Coalesce with next
    if (block->next && block->next->free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    // Coalesce with prev
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(block_header_t) + block->size;
        block->prev->next = block->next;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
}

void* realloc(void* ptr, uint32_t new_size) {
    if (!ptr) return malloc(new_size);

    if (!new_size) {
        free(ptr);
        return NULL;
    }

    uintptr_t user_ptr = (uintptr_t)ptr;
    block_header_t* block = *((block_header_t**)(user_ptr - sizeof(void*)));

    if (block->size >= new_size) {
        size_t excess = block->size - new_size;

        if (excess >= sizeof(block_header_t) + sizeof(void*)) {
            block_header_t* new_block = (block_header_t*)((uint8_t*)block + sizeof(block_header_t) + new_size + sizeof(void*));
            new_block->size = excess - sizeof(block_header_t) - sizeof(void*);
            new_block->free = true;
            new_block->next = block->next;
            new_block->prev = block;

            if (block->next) block->next->prev = new_block;
            block->next = new_block;
            block->size = new_size;
        }

        // No need to move since it was shrunk in-place
        return ptr;
    }

    // Old block is not large enough. 
    void* new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size);
    free(ptr);

    return new_ptr;
}

void* recalloc(void* ptr, uint32_t new_size) {
    ptr = realloc(ptr, new_size);
    memset(ptr, 0, new_size);
}