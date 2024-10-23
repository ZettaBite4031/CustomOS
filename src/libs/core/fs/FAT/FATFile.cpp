#include "FATFile.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Algorithm.hpp>
#include <core/cpp/Memory.hpp>

#include <fs/FATFileSystem.hpp>

FATFile::FATFile() 
    : m_FS(nullptr), m_Opened(false), m_IsRootDir(false), m_FirstCluster(), m_CurrentCluster(),
    m_CurrentSectorInCluster(), m_Position(), m_Size(), m_CurrentClusterIdx(), m_IsDirectory(false) {}

bool FATFile::Open(FATFileSystem* fs, uint32_t firstCluster, const char* name, uint32_t size, bool isDirectory) {
    m_FS = fs;

    m_IsDirectory = isDirectory;
    m_IsRootDir = false;
    m_Position = 0;
    m_Size = size;
    m_FirstCluster = firstCluster;
    m_CurrentCluster = m_FirstCluster;
    m_CurrentClusterIdx = 0;
    m_CurrentSectorInCluster = 0;
    
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
    fileEntry->Initialize(m_FS, dirEntry);

    return fileEntry;
}

void FATFile::Release() {
    m_FS->ReleaseFile(this);
}

size_t FATFile::Read(uint8_t* data, size_t count) {
    uint8_t* originalDataPtr = data;
    if (!m_IsDirectory || (m_IsDirectory && m_Size != 0))
        count = min(count, m_Size - m_Position);

    while (count > 0) {
        uint32_t leftInBuffer = SectorSize - (m_Position % SectorSize);
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

    return originalDataPtr - data;
}

size_t FATFile::Write(const uint8_t* data, size_t size) {
    return -1;
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
        m_Position = min(Size(), static_cast<uint32_t>(m_Position + rel));
        break;
    case SeekPos::End:
        if (rel < 0 && Size() < -rel)
            m_Position = 0;
        m_Position = min(Size(), static_cast<uint32_t>(Size() + rel));

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
