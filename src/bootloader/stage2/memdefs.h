#pragma once

// 0x00000000 - 0x000003FF : Interrupt Vector Table
// 0x00000400 - 0x000004FF : BIOS Data Region

#define MEMORY_MIN      0x00000500
#define MEMORY_MAX      0x00080000

// 0x00020000 - 0x0002FFFF : Stage2 FAT Driver
#define MEMORY_FAT_ADDR ((void*)0x20000)
#define MEMORY_FAT_SIZE 0x0000FFFF

// 0x00030000 - 0x0003FFFF : ELF load space
#define MEMORY_ELF_ADDR ((void*)0x30000)
#define MEMORY_ELF_SIZE 0xFFFF

// 0x00040000 - 0x0004FFFF : kernel load space
#define MEMORY_LOAD_KERNEL_ADDR ((void*)0x40000)
#define MEMORY_LOAD_KERNEL_SIZE 0x0000FFFF

// 0x00050000 - 0x00051FFF : VESA load space
#define MEMORY_VESA_INFO ((void*)0x50000)
#define MEMORY_VIDEO_MODE_INFO ((void*)0x51000)

// 0x00060000 - 0x0007FFFF : free

// 0x00080000 - 0x0009FFFF : Extended BIOS Data Region
// 0x000A0000 - 0x000C7FFF : Video 
// 0x000C8000 - 0x000FFFFF : BIOS 

#define MEMORY_KERNEL_ADDR ((void*)0x00100000)
