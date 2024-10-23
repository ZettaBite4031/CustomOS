#pragma once

#include <stdint.h>

#include <core/ZosDefs.hpp>

constexpr size_t SectorSize = 512;

enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY         = 0x01,
    FAT_ATTRIBUTE_HIDDEN            = 0x02,
    FAT_ATTRIBUTE_SYSTEM            = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID         = 0x08,
    FAT_ATTRIBUTE_DIRECTORY         = 0x10,
    FAT_ATTRIBUTE_ARCHIVE           = 0x20,
    FAT_ATTRIBUTE_LFN               = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};

struct FAT_DirectoryEntry {
    uint8_t Name[11];
    uint8_t Attributes;
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh;
    uint16_t ModifiedTime;
    uint16_t ModifiedDate;
    uint16_t FirstClusterLow;
    uint32_t Size;
} PACKED;

struct FAT_ExtendedBootRecord {
    // extended boot record
    uint8_t DriveNumber;
    uint8_t _Reserved;
    uint8_t Signature;
    uint32_t VolumeId;          // serial number, value doesn't matter
    uint8_t VolumeLabel[11];    // 11 bytes, padded with spaces
    uint8_t SystemId[8];
} PACKED;

struct FAT32_ExtendedBootRecord {
    uint32_t SectorsPerFat;
    uint16_t Flags;
    uint16_t FatVersion;
    uint32_t RootDirectoryCluster;
    uint16_t FSInfoSector;
    uint16_t BackupBootSector;
    uint8_t _Reserved[12];
    FAT_ExtendedBootRecord EBR;
} PACKED;

struct FAT_BootSector {
    uint8_t BootJumpInstruction[3];
    uint8_t OemIdentifier[8];
    uint16_t BytesPerSector;
    uint8_t SectorsPerCluster;
    uint16_t ReservedSectors;
    uint8_t FatCount;
    uint16_t DirEntryCount;
    uint16_t TotalSectors;
    uint8_t MediaDescriptorType;
    uint16_t SectorsPerFat;
    uint16_t SectorsPerTrack;
    uint16_t Heads;
    uint32_t HiddenSectors;
    uint32_t LargeSectorCount;

    union {
        FAT_ExtendedBootRecord EBR1216;
        FAT32_ExtendedBootRecord EBR32;
    };

    // ... we don't care about code ...
} PACKED;

struct FAT_LFN_Block{
    uint8_t Order;
    int16_t Chars[13]; 
};

