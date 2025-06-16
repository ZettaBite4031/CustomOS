#pragma once

#include <stdint.h>
#include <core/ZosDefs.hpp>

typedef struct {
    uint8_t Attributes;
    uint8_t CHS_StartAddress[3];
    uint8_t ParitionType;
    uint8_t CHS_LastPartitionAddress[3];
    uint32_t LBA_Start;
    uint32_t SectorCount;
} PACKED MBR_entry;