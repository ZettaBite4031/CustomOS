#pragma once

#include <stdint.h>

typedef struct {
    uint64_t Begin, Length;
    uint32_t Type;
    uint32_t ACPI;
} MemoryRegion;

typedef struct {
    int BlockCount;
    MemoryRegion* Regions;
} MemoryInfo;

typedef struct {
    MemoryInfo Memory;
    uint8_t BootDevice;
    void* PartitionLocation;
} BootParams;