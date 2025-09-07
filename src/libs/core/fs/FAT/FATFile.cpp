#include "FATFile.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Algorithm.hpp>
#include <core/cpp/Memory.hpp>

#include <fs/FATFileSystem.hpp>

FATFile::FATFile() 
    : m_FS(nullptr), m_Opened(false), m_IsRootDir(false), m_FirstCluster(), m_CurrentCluster(),
    m_CurrentSectorInCluster(), m_Position(), m_Size(), m_CurrentClusterIdx(), m_IsDirectory(false) {}

bool FATFile::Open(FATFileSystem* fs, uint32_t firstCluster, const char* name, uint32_t size, bool isDirectory, uint32_t parentDirCluster) {
    m_FS = fs;

    m_IsDirectory = isDirectory;
    m_IsRootDir = false;
    m_Position = 0;
    m_Size = size;
    m_FirstCluster = firstCluster;
    m_CurrentCluster = m_FirstCluster;
    m_CurrentClusterIdx = 0;
    m_CurrentSectorInCluster = 0;
    m_ParentDirCluster = parentDirCluster;
    
    if (!m_FS->ReadSectorFromCluster(m_CurrentCluster, m_Buffer, m_CurrentSectorInCluster)) {
        Debug::Error("FatFile", "Failed to open file!");
        return false;
    }

    m_Opened = true;
    return true;
}

bool FATFile::OpenRootDirectory1216(FATFileSystem* fs, uint32_t rootDirLba, uint32_t rootDirSize) {
    m_FS = fs;
    
    m_IsRootDir = true;
    m_Position = 0;
    m_Size = rootDirSize;
    m_FirstCluster = rootDirLba;
    m_CurrentCluster = m_FirstCluster;
    m_CurrentClusterIdx = 0;
    m_CurrentSectorInCluster = 0;
    
    if (!m_FS->ReadSector(rootDirLba, m_Buffer)) {
        Debug::Error("FatFile", "Failed to read root directory!\r\n");
        return false;
    }

    return true;
}

bool FATFile::ReadFileEntry(FAT_DirectoryEntry* dirEntry) {
    return Read(reinterpret_cast<uint8_t*>(dirEntry), sizeof(FAT_DirectoryEntry)) == sizeof(FAT_DirectoryEntry);
} 

FileEntry* FATFile::ReadFileEntry() {
    FAT_DirectoryEntry dirEntry;
    if (!ReadFileEntry(&dirEntry)) {
        Debug::Error("FatFile", "Failed to read directory entry!");
        return nullptr;
    }

    FATFileEntry* fileEntry = m_FS->AllocateFileEntry();
    if (!fileEntry) {
        Debug::Error("FatFile", "Failed to allocate a file entry!");
        return nullptr;
    }
    fileEntry->Initialize(m_FS, dirEntry, m_CurrentCluster);

    return fileEntry;
}

void FATFile::Release() {
    m_FS->ReleaseFile(this);
}

size_t FATFile::Read(uint8_t* data, size_t count) {
    uint8_t* originalDataPtr = data;
    if (!m_IsDirectory || (m_IsDirectory && m_Size != 0))
        count = min(count, (size_t)(m_Size - m_Position));

    while (count > 0) {
        size_t leftInBuffer = SectorSize - (m_Position % SectorSize);
        uint32_t take = min(count, leftInBuffer);

        Memory::Copy(data, m_Buffer + (m_Position % SectorSize), take);
        data += take;
        m_Position += take;
        count -= take;

        // check if there's more data
        if (leftInBuffer == take) {
            if (m_IsRootDir) {
                m_CurrentCluster++;
                if (!m_FS->ReadSector(m_CurrentCluster, m_Buffer)) {
                    Debug::Error("FatFile", "Failed to read FAT12/16 Root Directory!");
                    break;
                }
            } else {
                if (m_CurrentSectorInCluster++ >= m_FS->Data().BS.BootSector.SectorsPerCluster) {
                    m_CurrentSectorInCluster = 0;
                    m_CurrentCluster = m_FS->GetNextCluster(m_CurrentCluster);
                    m_CurrentClusterIdx++;
                }
                if (m_CurrentCluster >= 0xFFFFFFF8) {
                    // EOF
                    m_Size = m_Position;
                    break;
                }

                if (!m_FS->ReadSectorFromCluster(m_CurrentCluster, m_Buffer, m_CurrentSectorInCluster)) {
                    Debug::Error("FatFile", "Failed to read next sector!");
                    break;
                }
            }
        }
    }

    return data - originalDataPtr;
}

size_t FATFile::Write(const uint8_t* data, size_t count) {
    const uint8_t* originalDataPtr = data;
    uint32_t originalSize = m_Size;

    while (count > 0) {
        size_t offsetInSector = m_Position % SectorSize;
        size_t spaceInBuffer = SectorSize - offsetInSector;
        size_t toWrite = min(count, spaceInBuffer);

        // If we are writing partial sector, read the current sector to preserve unwritten bytes
        if (offsetInSector != 0 || toWrite != SectorSize) {
            if (!m_FS->ReadSectorFromCluster(m_CurrentCluster, m_Buffer, m_CurrentSectorInCluster)) {
                Debug::Error("FATFile", "Failed to read sector for partial write!");
                break;
            }
        }

        // Copy the data into the buffer
        Memory::Copy(m_Buffer + offsetInSector, data, toWrite);

        // Flush the buffer to disk ALWAYS after copying
        if (!m_FS->WriteSectorFromCluster(m_CurrentCluster, m_Buffer, m_CurrentSectorInCluster)) {
            Debug::Error("FATFile", "Failed to write sector!");
            break;
        }

        Debug::Info("FATFile", "Wrote %zu bytes to cluster %u sector %u pos %u",
                    toWrite, m_CurrentCluster, m_CurrentSectorInCluster, m_Position);

        m_Position += toWrite;
        data += toWrite;
        count -= toWrite;

        // Move to next sector if current sector is fully written
        if (offsetInSector + toWrite == SectorSize) {
            m_CurrentSectorInCluster++;
            if (m_CurrentSectorInCluster >= m_FS->Data().BS.BootSector.SectorsPerCluster) {
                m_CurrentSectorInCluster = 0;

                uint32_t nextCluster = m_FS->GetNextCluster(m_CurrentCluster);
                if (nextCluster >= 0xFFFFFFF8) {
                    // Need to allocate a new cluster and link it
                    uint32_t newCluster = m_FS->AllocateCluster();
                    if (!newCluster) {
                        Debug::Error("FATFile", "Failed to allocate new cluster!");
                        break;
                    }
                    if (!m_FS->LinkCluster(m_CurrentCluster, newCluster)) {
                        Debug::Error("FATFile", "Failed to link new cluster!");
                        break;
                    }
                    m_CurrentCluster = newCluster;

                    // Flush FAT table after modifying it
                    if (!m_FS->FlushFAT()) {
                        Debug::Error("FATFile", "Failed to flush FAT after cluster allocation!");
                        break;
                    }
                } else {
                    m_CurrentCluster = nextCluster;
                }
            }
        }

        // Update the file size in memory
        if (m_Position > m_Size) {
            m_Size = m_Position;
        }
    }

    // After writing, update the directory entry on disk if size changed
    if (m_Size != originalSize) {
        if (!m_FS->UpdateFileEntrySize(this, m_Size)) {
            Debug::Error("FATFile", "Failed to update directory entry file size!");
        }
    }

    return data - originalDataPtr;
}


bool FATFile::Seek(int rel, SeekPos pos) {
    switch (pos)
    {
    case SeekPos::Set: 
        m_Position = static_cast<uint32_t>(max(0, rel));
        break;
    case SeekPos::Current:
        if (rel < 0 && m_Position < -rel)
            m_Position = 0;
        m_Position = min(Size(), static_cast<size_t>(m_Position + rel));
        break;
    case SeekPos::End:
        if (rel < 0 && Size() < -rel)
            m_Position = 0;
        m_Position = min(Size(), static_cast<size_t>(Size() + rel));

    default:
        break;
    }

    return UpdateCurrentCluster();
}

bool FATFile::UpdateCurrentCluster() {
    uint32_t clusterSize = m_FS->Data().BS.BootSector.SectorsPerCluster;
    uint32_t desiredCluster = m_Position / clusterSize;
    uint32_t desiredSector = (m_Position % clusterSize) / SectorSize;

    if (desiredCluster == m_CurrentClusterIdx && desiredSector == m_CurrentSectorInCluster) return true;

    if (desiredCluster < m_CurrentClusterIdx) {
        m_CurrentClusterIdx = 0;
        m_CurrentCluster = m_FirstCluster;
    }
    while (desiredCluster > m_CurrentClusterIdx) {
        m_CurrentCluster = m_FS->GetNextCluster(m_CurrentCluster);
        m_CurrentClusterIdx++;
    }

    m_CurrentSectorInCluster = desiredSector;
    return m_FS->ReadSectorFromCluster(m_CurrentCluster, m_Buffer, m_CurrentSectorInCluster);
}

bool FATFile::Resize(size_t size) {
    if (!m_Opened || m_IsDirectory) {
        Debug::Error("FATFile", "TruncateTo called on a directory or unopened file.");
        return false;
    }

    uint32_t clusterSizeBytes = m_FS->Data().BS.BootSector.SectorsPerCluster * SectorSize;
    uint32_t currentClusterCount = (m_Size + clusterSizeBytes - 1) / clusterSizeBytes;
    uint32_t desiredClusterCount = (size + clusterSizeBytes - 1) / clusterSizeBytes;

    if (desiredClusterCount == 0) {
        m_FS->FreeClusterChain(m_FirstCluster);

        m_CurrentCluster = m_FirstCluster = 0;
        m_CurrentClusterIdx = 0;
        m_CurrentSectorInCluster = 0;
        m_Position = 0;
        m_Size = 0;

        return true;
    }

    // Shrink file (removed unused cluster)
    if (desiredClusterCount < currentClusterCount) {
        uint32_t cluster = m_FirstCluster;
        for (uint32_t i = 0; i < desiredClusterCount - 1; i++) {
            cluster = m_FS->GetNextCluster(cluster);
        }

        uint32_t lastValid = cluster;
        uint32_t toFree = m_FS->GetNextCluster(cluster);

        if (!m_FS->SetNextCluster(lastValid, 0xFFFFFFFF)){
            Debug::Error("FATFile", "Failed to mark last valid character");
            return false;
        }

        while (toFree < 0xFFFFFFF8) {
            uint32_t next = m_FS->GetNextCluster(toFree);
            m_FS->FreeCluster(toFree);
            toFree = next;
        }
    }

    else if (desiredClusterCount > currentClusterCount) {
        uint32_t cluster = m_FirstCluster;
        while (cluster < 0xFFFFFFF8) {
            cluster = m_FS->GetNextCluster(cluster);
        }

        for (uint32_t i = currentClusterCount; i < desiredClusterCount; i++) {
            uint32_t newCluster = m_FS->AllocateCluster();
            if (!newCluster || !m_FS->LinkCluster(cluster, newCluster)) {
                Debug::Error("FATFile", "Failed to allocate/link cluster while resizing file.");
                return false;
            }
            cluster = newCluster;
        }
    }

    m_Size = size;
    if (m_Position > size) m_Position = size;

    return true;
}

bool FATFile::EraseContents() {
    if (!Resize(0))
        return false;
    
    Memory::Set(m_Buffer, 0, SectorSize);
    return m_FS->WriteSectorFromCluster(m_FirstCluster, m_Buffer, 0);
}