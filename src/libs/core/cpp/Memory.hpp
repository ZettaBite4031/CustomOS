#pragma once
#include <stdint.h>
#include <stddef.h>
#include <core/ZosDefs.hpp>
#include <boot/bootparams.h>

EXPORT void* ASMCALL memcpy(void* dst, const void* src, size_t size);
EXPORT void* ASMCALL memset(void* dst, uint32_t val, size_t size);
EXPORT int   ASMCALL memcmp(const void* ptr1, const void* ptr2, size_t size); 

namespace Memory {
    constexpr auto Copy = memcpy;
    constexpr auto Set = memset;
    constexpr auto Compare = memcmp;
}

#define ALIGN_UP(ptr, alignment) (((ptr) + ((alignment) - 1)) & ~((alignment) - 1))

uint32_t get_alignment(void* ptr);

bool Mem_Init(MemoryInfo* mem_info);
void* malloc(uint32_t size);
void* calloc(uint32_t size);
void free(void* ptr);
void* realloc(void* ptr, uint32_t new_size);
void* recalloc(void* ptr, uint32_t new_size);

void* malloc_aligned(uint32_t size, uint32_t alignment);
void free_aligned(void* ptr);