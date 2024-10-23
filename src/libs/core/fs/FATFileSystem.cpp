#include "FATFileSystem.hpp"

#include "FAT/FATHeaders.hpp"

#include <core/ZosDefs.hpp>
#include <core/Debug.hpp>

constexpr const char* LogModule = "FAT";

FATFileSystem::FATFileSystem() 
    : m_Device(nullptr), m_Data(new FAT_Data()),
      m_DataSectionLBA(0), m_FatType(0), m_SectorsPerFat(0), m_TotalSectors(0) {}

bool FATFileSystem::Initialize(BlockDevice* device) {
    m_Device = device;

    if (!ReadBootSector()) {
        Debug::Error(LogModule, "Failed to read BootSector!!");
        return false;
    }

    bool isFat32 = false;
    m_SectorsPerFat = m_Data->BS.BootSector.SectorsPerFat;
    if (m_SectorsPerFat == 0) {
        // fat32
        m_SectorsPerFat = m_Data->BS.BootSector.EBR32.SectorsPerFat;
        isFat32 = true;
    }  

    // open root directory file
    uint32_t rootDirLba;
    uint32_t rootDirSize = 0;
    if (isFat32) {
        m_DataSectionLBA = m_Data->BS.BootSector.ReservedSectors + m_SectorsPerFat * m_Data->BS.BootSector.FatCount;     
        if (!m_Data->RootDirectory.Open(this, m_Data->BS.BootSector.EBR32.RootDirectoryCluster, "", 0, true)) {
            Debug::Error(LogModule, "Failed to read Root Directory!");
            return false;
        }
    } else {
        rootDirLba = m_Data->BS.BootSector.ReservedSectors + m_SectorsPerFat * m_Data->BS.BootSector.FatCount;
        rootDirSize = sizeof(FAT_DirectoryEntry) * m_Data->BS.BootSector.DirEntryCount;
        uint32_t rootDirSectors = (rootDirSize + m_Data->BS.BootSector.BytesPerSector - 1) / m_Data->BS.BootSector.BytesPerSector;
        m_DataSectionLBA = rootDirLba + rootDirSectors;
        
        if (!m_Data->RootDirectory.OpenRootDirectory1216(this, rootDirLba, rootDirSize)) {
            Debug::Error(LogModule, "Failed to read Root Directory!");
            return false;
        }
    }
    Debug::Debug(LogModule, "Successfully opened the root directory!");

    DetectFatType();

    m_Data->LFN_Count = 0;

    if (m_Data->BS.BootSector.BytesPerSector != SectorSize) {
        Debug::Error(LogModule, "Bytes Per Sector != 512! Actual: %d", m_Data->BS.BootSector.BytesPerSector);
        return false;
    }

    return true;
}

File* FATFileSystem::RootDirectory() {
    return &m_Data->RootDirectory;
}

void FATFileSystem::DetectFatType() {
    if (m_Data->BS.BootSector.SectorsPerCluster == 0) {
        Debug::Critical(LogModule, "Sectors Per Cluster == 0!");
        HALT
    }
    uint32_t dataClusters = (m_TotalSectors - m_DataSectionLBA) / m_Data->BS.BootSector.SectorsPerCluster;
    if (dataClusters < 0xFF5) 
        m_FatType = 12;
    else if (m_Data->BS.BootSector.SectorsPerFat != 0) 
        m_FatType = 16;
    else m_FatType = 32;
}

bool FATFileSystem::ReadBootSector() {
    m_Device->Seek(0, SeekPos::Set);
    size_t read = m_Device->Read(m_Data->BS.BootSectorBytes, SectorSize);
    if (read != SectorSize) {
        Debug::Debug(LogModule, "Boot Sector read failed! Expected: %lu, Actual: %lu", SectorSize, read);
        return false;
    }
    return true;
}

bool FATFileSystem::ReadSector(uint32_t LBA, uint8_t* buffer, size_t count) {
    m_Device->Seek(LBA * SectorSize, SeekPos::Set);
    size_t read = m_Device->Read(buffer, count * SectorSize);
    size_t expected = count * SectorSize;
    if (read != expected) {
        Debug::Debug(LogModule, "Read Sector failed! Expected: %lu, Actual: %lu", expected, read);
        return false;
    }
    return true;
}

bool FATFileSystem::ReadSectorFromCluster(uint32_t cluster, uint8_t* buffer, size_t offset) {
    return ReadSector(ClusterToLBA(cluster) + offset, buffer);
}

uint32_t FATFileSystem::ClusterToLBA(uint32_t cluster) {
    return m_DataSectionLBA + (cluster - 2) * m_Data->BS.BootSector.SectorsPerCluster;
}

uint32_t FATFileSystem::GetNextCluster(uint32_t currentCluster) {   
    // determine the byte offset of the entry being read 
    uint32_t fatIndex;
    if (m_FatType == 12) {
        fatIndex = currentCluster * 3 / 2;
    } else if (m_FatType  == 16) {
        fatIndex = currentCluster * 2;
    } else /* if (m_FatType  == 32) */ {
        fatIndex = currentCluster * 4;
    }

    // Make sure cache has the desired sector
    uint32_t fatIndexSector = fatIndex / SectorSize;
    if (fatIndexSector < m_Data->FAT_CachePosition || fatIndexSector >= (m_Data->FAT_CachePosition + FatCacheSize)) {
        // Cache miss! Read entry from disk
        ReadFat(fatIndexSector);
        m_Data->FAT_CachePosition = fatIndexSector;
    }

    fatIndex -= (m_Data->FAT_CachePosition * SectorSize);

    uint32_t nextCluster;
    if (m_FatType  == 12) {
        if (currentCluster % 2 == 0)
            nextCluster = (*(uint16_t*)(m_Data->FAT_Cache + fatIndex)) & 0x0FFF;
        else nextCluster = (*(uint16_t*)(m_Data->FAT_Cache + fatIndex)) >> 4;
        if (nextCluster >= 0xFF8)
            nextCluster |= 0xFFFFF000;
    } else if (m_FatType  == 16) {
        nextCluster = *(uint16_t*)(m_Data->FAT_Cache + fatIndex);
        if (nextCluster >= 0xFFF8)
            nextCluster |= 0xFFFF0000;
    } else /* if (m_FatType  == 32) */ {
        nextCluster = *(uint32_t*)(m_Data->FAT_Cache + fatIndex);
    }
    return nextCluster;
}

bool FATFileSystem::ReadFat(uint32_t lbaOffset) {
    return ReadSector(m_Data->BS.BootSector.ReservedSectors + lbaOffset, m_Data->FAT_Cache, FatCacheSize);
}

FATFile* FATFileSystem::AllocateFile() {
    return m_Data->OpenedFilePool.Allocate();
}

void FATFileSystem::ReleaseFile(FATFile* file) {
    m_Data->OpenedFilePool.Free(file);
}

FATFileEntry* FATFileSystem::AllocateFileEntry() {
    return m_Data->FileEntryPool.Allocate();
}

void FATFileSystem::ReleaseFileEntry(FATFileEntry* entry) {
    m_Data->FileEntryPool.Free(entry);
}
