#pragma once

#define EXPORT extern "C"

#define ASMCALL __attribute__((cdecl))

// Short-hand macro for marking a structure as packed to avoid any padding issues.
#define PACKED __attribute__((packed))

// Short-hand macro for obtaining the size of an array.
#define _countof(array) (sizeof((array))/sizeof(((array)[0])))

// Short-hand macro for halting the CPU.
#define HALT for(;;);
