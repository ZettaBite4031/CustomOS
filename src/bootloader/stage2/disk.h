#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t id;
    uint8_t type;
    uint16_t cylinder;
    uint16_t sectors;
    uint16_t heads;
} DISK;

bool DISK_Initialize(DISK* disk, uint8_t driveNumber);
bool DISK_ReadSectors(DISK* disk, uint32_t LBA, uint8_t sectors, uint8_t* data);
