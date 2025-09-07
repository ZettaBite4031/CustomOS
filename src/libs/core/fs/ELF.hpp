#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/ZosDefs.hpp>

#include "File.hpp"

typedef struct ELFHeader {
    uint8_t Magic[4];
    uint8_t Bitness;        // 1 = 32 bit, 2 = 64 bit
    uint8_t Endianness;     // 1 = little, 2 = big
    uint8_t HeaderVersion;
    uint8_t ABI;
    uint8_t _Reserved[8];
    uint16_t ELFType;       // 1 = relocatable, 2 = executable, 3 = shared, 4 = core
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
} PACKED ELFHeader_t;

typedef struct ELF32ProgramHeader {
    uint32_t Type;
    uint32_t Offset;
    uint32_t VirtAddr;
    uint32_t PhysAddr;
    uint32_t Filesize;
    uint32_t MemorySize;
    uint32_t Flags;
    uint32_t Align;     // 0 and 1 specify no alignment; otherwise, should be positive integral power of 2
                        // VirtAddr should be equal to Offset % Align 
} PACKED ELF32ProgramHeader_t;

enum class ELFBitness : uint8_t {
    BITS32      = 1,
    BITS64      = 2,
};

enum class ELFEndianness : uint8_t {
    LITTLE      = 1,
    BIG         = 2,
};

enum class ELFType : uint16_t {
    RELOCATABLE = 1,
    EXECUTABLE  = 2,
    SHARED      = 3,
    CORE        = 4,
};

enum class ELFInstructionSet : uint16_t {
    NONSPECIFIC = 0x00,
    SPARC       = 0x02,
    X86         = 0x03,
    MIPS        = 0x08,
    POWERPC     = 0x14,
    ARM         = 0x28,
    SUPERH      = 0x2A,
    IA64        = 0x32,
    X86_64      = 0x3E,
    AARCH64     = 0xB7,
    RISCV       = 0xF3,
}; // Currently, only x86 and x86_64 really matter, but the rest are included for completeness. Sourced from the OSDev wiki.

enum class ELFProgramHeaderType : uint32_t {
    UNUSED      = 0x00, // Program header type entry unused
    LOAD        = 0x01, // Loadable segment
    DYNAMIC     = 0x02, // Dynamic linking information
    INTERP      = 0x03, // Interpreter information
    NOTE        = 0x04, // Auxiliary information
    SHLIB       = 0x05, // Reserved
    PHDR        = 0x06, // Program Header segment
    TLS         = 0x07, // Thread-Local Storage template
};

enum class ELFProgramHeaderFlags : uint32_t {
    EXECUTABLE  = 0x01,
    WRITABLE    = 0x02,
    READABLE    = 0x04,
    RWE         = EXECUTABLE | WRITABLE | READABLE,
};

class ELF {
public:
    constexpr static uint8_t ELF_MAGIC[4]{ '\x7F', '\x45', '\x4C', '\x46' };
    ELF(File* elf_file_handle);
private:
    bool ValidateHeader();

    ELFHeader_t m_Header{ 0 };
};