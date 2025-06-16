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

template<typename T>
uint32_t ToSegOffset(T addr) {
    uint32_t addr32 = reinterpret_cast<uint32_t>(addr);
    uint32_t segment = (addr32 >> 4) & 0xFFFF;
    uint32_t offset = addr32 & 0xF;
    return (segment << 16) | offset; 
}

template<typename T>
T ToLinear(uint32_t segoff) {
    uint32_t offset = (uint32_t)(segoff) & 0xFFFF;
    uint32_t segment = (uint32_t)(segoff) >> 16;
    return (T)(segment * 16 + offset);
}