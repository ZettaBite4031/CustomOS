#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "mbr.h"
#include "zosdefs.h"

#define ELF_MAGIC "\x7F\x45\x4C\x46"

typedef struct {
    uint8_t Magic[4];
    uint8_t Bitness;            // 1 = 32 bit, 2 = 64 bit
    uint8_t Endianness;         // 1 = little, 2 = big
    uint8_t HeaderVersion;
    uint8_t ABI;
    uint8_t _Reserved[8];
    uint16_t ELFType;             // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
    uint16_t InstructionSet;
    uint32_t ELFVersion;
    uint32_t ProgramEntryPosition;
    uint32_t ProgramHeaderTablePosition;
    uint32_t SectionHeaderTablePosition;
    uint32_t Flags;
    uint16_t HeaderSize;
    uint16_t ProgramHeaderTableEntrySize;
    uint16_t ProgramHeaderTableEntryCount;
    uint16_t SectionHeaderTableEntrySize;
    uint16_t SectionHeaderTableEntryCount;
    uint16_t SectionNamesIndex;
} PACKED ELFHeader;

typedef struct {
    uint32_t Type;
    uint32_t Offset;
    uint32_t VirtAddr;
    uint32_t PhysAddr;
    uint32_t FileSize;
    uint32_t MemorySize;
    uint32_t Flags;
    uint32_t Align;         // 0 and 1 specify no alignment; otherwise, should be a positive integral power of 2
                            // VirtAddr should equal Offset % Align
} PACKED ELF32ProgramHeader;

enum ELFBitness {
    ELF_BITNESS_32BIT               = 1,
    ELF_BITNESS_64BIT               = 2
};

enum ELFEndianness {
    ELF_ENDIANNESS_LITTLE           = 1,
    ELF_ENDIANNESS_BIG              = 2
};

enum ELFType {
    ELF_TYPE_RELOCATABLE            = 1,
    ELF_TYPE_EXECUTABLE             = 2,
    ELF_TYPE_SHARED                 = 3,
    ELF_TYPE_CORE                   = 4
};

enum ELFInstructionSet {
    ELF_INSTRUCTIONSET_NONSPECIFIC  = 0x00,
    ELF_INSTRUCTIONSET_SPARC        = 0x02,
    ELF_INSTRUCTIONSET_X86          = 0x03,     
    ELF_INSTRUCTIONSET_MIPS         = 0x08,
    ELF_INSTRUCTIONSET_POWERPC      = 0x14,
    ELF_INSTRUCTIONSET_ARM          = 0x28,
    ELF_INSTRUCTIONSET_SUPERH       = 0x2A,
    ELF_INSTRUCTIONSET_IA64         = 0x32,
    ELF_INSTRUCTIONSET_X86_64       = 0x3E,     
    ELF_INSTRUCTIONSET_AARCH64      = 0xB7,
    ELF_INSTRUCTIONSET_RISCV        = 0xF3
}; // Currently, only x86 and x86_64 really matter, but the rest are included for completeness. Sourced from the OSDev wiki.

enum ELFProgramHeaderType {
    ELF_PROGRAM_HEADER_TYPE_NULL        = 0x00, // Program header type entry unused
    ELF_PROGRAM_HEADER_TYPE_LOAD        = 0x01, // Loadable segment
    ELF_PROGRAM_HEADER_TYPE_DYNAMIC     = 0x02, // Dynamic linking information
    ELF_PROGRAM_HEADER_TYPE_INTERP      = 0x03, // Interpreter information
    ELF_PROGRAM_HEADER_TYPE_NOTE        = 0x04, // Auxiliary information
    ELF_PROGRAM_HEADER_TYPE_SHLIB       = 0x05, // Reserved
    ELF_PROGRAM_HEADER_TYPE_PHDR        = 0x06, // Program Header segment
    ELF_PROGRAM_HEADER_TYPE_TLS         = 0x07, // Thread-Local Storage template
};

enum ELFProgramHeaderFlags {
    ELF_PROGRAM_HEADER_FLAGS_EXECUTABLE = 0x01,
    ELF_PROGRAM_HEADER_FLAGS_WRITABLE   = 0x02,
    ELF_PROGRAM_HEADER_FLAGS_READABLE   = 0x04,
    ELF_PROGRAM_HEADER_FLAGS_RWE        = ELF_PROGRAM_HEADER_FLAGS_READABLE | ELF_PROGRAM_HEADER_FLAGS_WRITABLE | ELF_PROGRAM_HEADER_FLAGS_EXECUTABLE
};

bool ELF_Read(Partition* partition, const char* path, void** entryPoint); 
