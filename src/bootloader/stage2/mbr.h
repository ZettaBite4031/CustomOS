#pragma once
#include <stdint.h>
#include <stdbool.h>

#include "disk.h"

typedef struct {
    DISK* disk;
    uint32_t PartitionOffset;
    uint32_t PartitionSize;
} Partition;

void MBR_DetectPartition(Partition* partition, DISK* disk, void* partition_location);

bool Partition_ReadSectors(Partition* partition, uint32_t lba, uint8_t sectors, void* dataOut);