#include "mbr.h"
#include "memory.h"
#include "memdefs.h"
#include "stdio.h"

typedef struct {
    uint8_t Attributes;
    uint8_t CHS_StartAddress[3];
    uint8_t PartitionType;
    uint8_t CHS_LastPartitionAddress[3];
    uint32_t LBA_Start;
    uint32_t SectorCount;
} __attribute__((packed)) MBR_Entry;

void MBR_DetectPartition(Partition* partition, DISK* disk, void* partition_location) {
    partition->disk = disk;
    if (disk->id < 0x80) {
        partition->PartitionOffset = 0;
        partition->PartitionSize = (uint32_t)(disk->cylinder) * (uint32_t)(disk->heads) * (uint32_t)(disk->sectors);
    } else {
        MBR_Entry* entry = (MBR_Entry*)SEGOFF2LIN(partition_location);
        partition->PartitionOffset = entry->LBA_Start;
        partition->PartitionSize = entry->SectorCount;
    }
}

bool Partition_ReadSectors(Partition* partition, uint32_t lba, uint8_t sectors, void* dataOut) {
    return DISK_ReadSectors(partition->disk, lba + partition->PartitionOffset, sectors, dataOut);
}
