#pragma once

#include "FileSystem.hpp"
#include "FAT/FATData.hpp"

constexpr size_t FATRequiredMemory = 0x10000;

class FATFileSystem : public FileSystem {
public:
    FATFileSystem();
    virtual bool Initialize(BlockDevice* device) override;
    virtual File* RootDirectory() override;

    bool ReadSector(uint32_t LBA, uint8_t* buffer, size_t count = 1);
    bool ReadSectorFromCluster(uint32_t cluster, uint8_t* buffer, size_t offset);
    uint32_t GetNextCluster(uint32_t currentCluster);

    uint8_t FatType() const { return m_FatType; }
    FAT_Data& Data() { return *m_Data; }

    FATFile* AllocateFile();
    void ReleaseFile(FATFile* file);

    FATFileEntry* AllocateFileEntry();
    void ReleaseFileEntry(FATFileEntry* entry);

private:
    bool ReadBootSector();
    void DetectFatType();
    uint32_t ClusterToLBA(uint32_t cluster);
    bool ReadFat(uint32_t lbaOffset);

    BlockDevice* m_Device;
    FAT_Data* m_Data;
    uint32_t m_DataSectionLBA;
    uint8_t m_FatType;
    uint32_t m_TotalSectors;
    uint32_t m_SectorsPerFat;
};
