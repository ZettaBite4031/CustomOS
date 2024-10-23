#pragma once

#include <stddef.h>

// 0x00000000 - 0x000003FF : Interrupt Vector Table
// 0x00000400 - 0x000004FF : BIOS Data Region

constexpr size_t MemoryMin = 0x00020000;
constexpr size_t MemoryMax = 0x00080000;
// 0x00020000 - 0x0007FFFF : Stage2Allocator

// 0x00080000 - 0x0009FFFF : Extended BIOS Data Region
// 0x000A0000 - 0x000C7FFF : Video 
// 0x000C8000 - 0x000FFFFF : BIOS 

#define MEMORY_KERNEL_ADDR ((void*)0x00100000)
