#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "zosdefs.h"

void EXTERN x86_OutB(uint16_t port, uint8_t value);
uint8_t EXTERN x86_InB(uint16_t port);

bool EXTERN x86_DiskReset(uint8_t drive);
bool EXTERN x86_DiskRead(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t* lowerDataOut);
bool EXTERN x86_GetDiskDriveParams(uint8_t drive, uint8_t* driveType, uint16_t* cylinders, uint16_t* sectors, uint16_t* heads);

int EXTERN x86_Video_GetVbeInfo(void* infoOut);
int EXTERN x86_Video_GetModeInfo(uint16_t mode, void* infoOut);
int EXTERN x86_Video_SetMode(uint16_t mode);

typedef struct {
    uint64_t Base;
    uint64_t Length;
    uint32_t Type;
    uint32_t ACPI;
} E820MemoryBlock;

enum E820MemoryBlockType {
    E820_USABLE = 1,
    E820_RESERVED = 2,
    E820_ACPI_RECLAIMABLE = 3,
    E820_ACPI_NVS = 4,
    E820_BAD_MEMORY = 5,
};

int EXTERN x86_E820GetNextBlock(E820MemoryBlock* block, uint32_t* continuationId);
