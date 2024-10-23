#include "elf.h"
#include "fat.h"
#include "memory.h"
#include "memdefs.h"
#include "stdio.h"
#include "debug.h"
#include "utility.h"

bool ELF_ValidateElfHeader(ELFHeader* header) {
    bool ok = true;
    if (memcmp(header->Magic, ELF_MAGIC, 4) != 0) {
        LogError("ELF", "Read file's magic is not the ELF Magic!");
        printf("The read file's magic is not the ELF Magic!\r\n");
        ok = false;
    }
    if (header->Bitness != ELF_BITNESS_32BIT) {
        LogError("ELF", "ELF is not 32-bit!");
        printf("ELF is not 32-bit!\r\n");
        ok = false;
    }
    if (header->Endianness != ELF_ENDIANNESS_LITTLE) {
        LogError("ELF", "ELF is not little-endian!");
        printf("ELF is not little-endian!\r\n");
        ok = false;
    }
    if (header->ELFVersion != 1) { 
        LogError("ELF", "ELF Version is off. Expected: 1 | Actual: %d", header->ELFVersion);
        printf("ELF Version is off. Expected: 1 | Actual: %d\r\n", header->ELFVersion);
        ok = false;
    }
    if (header->HeaderVersion != 1) {
        LogError("ELF", "ELF Header Version is off. Expected: 1 | Actual: %d", header->ELFVersion);
        printf("ELF Header Version is off. Expected: 1 | Actual: %d\r\n", header->ELFVersion);
        ok = false;
    }
    if (header->ELFType != ELF_TYPE_EXECUTABLE) {
        LogError("ELF", "ELF Type is incorrect! Expected: %d | Actual: %d", ELF_TYPE_EXECUTABLE, header->ELFType);
        printf("ELF Type is incorrect! Expected: %d | Actual: %d", ELF_TYPE_EXECUTABLE, header->ELFType);
        ok = false;
    }
    if (header->InstructionSet != ELF_INSTRUCTIONSET_X86) {
        LogError("ELF", "ELF Instruction Set is incorrect! Expected: %d | Actual: %d", ELF_INSTRUCTIONSET_X86, header->InstructionSet);
        printf("ELF Instruction Set is incorrect! Expected: %d | Actual: %d", ELF_INSTRUCTIONSET_X86, header->InstructionSet);
        ok = false;
    }
    return ok;
}

bool ELF_Read(Partition* partition, const char* path, void** entryPoint) {
    FAT_File* fd = FAT_Open(partition, path);

    uint8_t* headerBuffer = (uint8_t*)MEMORY_ELF_ADDR;
    uint8_t* loadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL_ADDR;

    uint32_t filePos = 0;
    uint32_t read;
    
    if ((read = FAT_Read(partition, fd, sizeof(ELFHeader), headerBuffer)) != sizeof(ELFHeader)) {
        LogCritical("ELF", "Failed to read ELF!");
        printf("ELF: Failed to read ELF!\n");
        return false;
    }
    filePos += read;

    ELFHeader* header = (ELFHeader*)headerBuffer;

    if (!ELF_ValidateElfHeader(header)) {
        LogError("ELF", "Failed to validate the ELF Header!");
        printf("Failed to validate the ELF Header!");
        return false;
    }

    // load program header
    *entryPoint = (void*)header->ProgramEntryPosition;
    uint32_t programHeaderOffset = header->ProgramHeaderTablePosition;
    uint32_t programHeaderEntrySize = header->ProgramHeaderTableEntrySize;
    uint32_t programHeaderEntryCount = header->ProgramHeaderTableEntryCount;
    uint32_t programHeaderSize = programHeaderEntryCount * programHeaderEntrySize;

    // `header` variable invalidated.
    filePos += FAT_Read(partition, fd, programHeaderOffset - filePos, headerBuffer);
    if ((read = FAT_Read(partition, fd, programHeaderSize, headerBuffer) != programHeaderSize)) {
        LogError("ELF", "Failed to read the program header table!");
        printf("Failed to read the program header table!\r\n");
        return false;
    }
    filePos += read;
    FAT_Close(fd);

    // parse program header entries
    for (uint32_t i = 0; i < programHeaderEntryCount; i++) {
        ELF32ProgramHeader* programHeader = (ELF32ProgramHeader*)(headerBuffer + (i * programHeaderEntrySize));
        if (programHeader->Type == ELF_PROGRAM_HEADER_TYPE_LOAD) {
            // Load !
            // TODO: Validate the program doesn't overwrite Stage2
            uint8_t* virtAddr = (uint8_t*)programHeader->VirtAddr;
            memset(virtAddr, 0, programHeader->MemorySize);

            // ugly nasty seeking hack
            // TODO: Proper seeking !
            fd = FAT_Open(partition, path);
            while (programHeader->Offset > 0) {
                uint32_t desiredRead = min(programHeader->Offset, MEMORY_LOAD_KERNEL_SIZE);
                read = FAT_Read(partition, fd, desiredRead, loadBuffer);
                if (read != desiredRead) {
                    LogError("ELF", "Failed to load kernel program!");
                    printf("ELF: Failed to load kernel program!");
                    return false;
                }
                programHeader->Offset -= read;
            }

            // read program
            while (programHeader->FileSize > 0) {
                uint32_t desiredRead = min(programHeader->FileSize, MEMORY_LOAD_KERNEL_SIZE);
                read = FAT_Read(partition, fd, desiredRead, loadBuffer);
                if (read != desiredRead) {
                    LogError("ELF", "Failed to read kernel program!");
                    printf("ELF: Failed to read kernel program!");
                    return false;
                }
                programHeader->FileSize -= read;

                memcpy(virtAddr, loadBuffer, read);
                virtAddr += read;
            }
            FAT_Close(fd);
        }
    }

    return true;
}