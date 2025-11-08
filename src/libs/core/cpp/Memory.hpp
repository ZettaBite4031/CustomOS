#pragma once
#include <stdint.h>
#include <stddef.h>
#include <core/ZosDefs.hpp>
#include <boot/bootparams.h>


EXPORT void* ASMCALL memcpy(void* dst, const void* src, size_t size);
EXPORT void* ASMCALL memset(void* dst, uint32_t val, size_t size);
EXPORT int   ASMCALL memcmp(const void* ptr1, const void* ptr2, size_t size); 
EXPORT void* ASMCALL memmove(void *, const void *, unsigned long);


namespace Memory {
    constexpr auto Copy = memcpy;
    constexpr auto Set = memset;
    constexpr auto Compare = memcmp;
    constexpr auto Move = memmove;
}

#define ALIGN_UP(ptr, alignment) (((ptr) + ((alignment) - 1)) & ~((alignment) - 1))

uint32_t get_alignment(void* ptr);

MemoryRegion FindBestRegion(MemoryInfo* info, size_t& idx);
bool Mem_Init(uintptr_t heap_base, const MemoryRegion& best_region);
void* zmalloc(uint32_t size);
void* zcalloc(uint32_t size, uint8_t n);
void zfree(void* ptr);
void* zrealloc(void* ptr, uint32_t new_size);
void* zrecalloc(void* ptr, uint32_t new_size);

void* zmalloc_aligned(uint32_t size, uint32_t alignment);

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