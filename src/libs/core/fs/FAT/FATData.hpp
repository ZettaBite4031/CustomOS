#pragma once
#include "FATHeaders.hpp"
#include "FATFile.hpp"
#include "FATFileEntry.hpp"

#include <core/mem/StaticObjectPool.hpp>

constexpr size_t MaxFileNameSize = 256;
constexpr size_t MaxFileHandles = 16;
constexpr size_t FatCacheSize = 5;
constexpr int32_t RootDirectoryHandle = -1;
constexpr uint32_t FAT_LFN_Last = 0x40;

struct FAT_Data {
    union {
        FAT_BootSector BootSector;
        uint8_t BootSectorBytes[SectorSize];
    } BS;

    FATFile RootDirectory;
    StaticObjectPool<FATFile, MaxFileHandles> OpenedFilePool;
    StaticObjectPool<FATFileEntry, MaxFileHandles> FileEntryPool;

    uint8_t FAT_Cache[FatCacheSize * SectorSize];
    uint32_t FAT_CachePosition;

    FAT_LFN_Block LFN_Blocks[FAT_LFN_Last];
    int LFN_Count;
};
