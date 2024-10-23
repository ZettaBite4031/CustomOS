#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    disk->id = driveNumber;
    if (!x86_GetDiskDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads)) return false;

    disk->type = driveType;
    disk->cylinder = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;
    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t LBA, uint16_t* cylinder, uint16_t* sector, uint16_t* head) {
    *sector = LBA % disk->sectors + 1;
    *cylinder = (LBA / disk->sectors) / disk->heads;
    *head = (LBA / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK* disk, uint32_t LBA, uint8_t sectors, uint8_t* data) {
    uint16_t cylinder, sector, head;
    DISK_LBA2CHS(disk, LBA, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++){
        if (x86_DiskRead(disk->id, cylinder, sector, head, sectors, data))
            return true;
        x86_DiskReset(disk->id);
    }
    return false;
}
