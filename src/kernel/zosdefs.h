#pragma once

// Short-hand macro which designates functions defined in assembly files.
#define EXTERN __attribute__((cdecl)) 

// Short-hand macro which desginates symbols in C as visible to assembly files.
#define GLOBAL __attribute__((cdecl))

// Short-hand macro for marking a structure as packed to avoid any padding issues.
#define PACKED __attribute__((packed))

// Short-hand macro for obtaining the size of an array.
#define _countof(array) (sizeof((array))/sizeof(((array)[0])))

// Short-hand macro for halting the CPU.
#define HALT for(;;);
