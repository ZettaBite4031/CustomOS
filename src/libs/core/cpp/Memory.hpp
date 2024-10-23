#pragma once
#include <stdint.h>
#include <stddef.h>
#include <core/ZosDefs.hpp>

EXPORT void ASMCALL memcpy(void* dst, const void* src, size_t size);
EXPORT void ASMCALL memset(void* dst, uint32_t val, size_t size);

namespace Memory {
    constexpr auto Copy = memcpy;
    constexpr auto Set = memset;
}
