#pragma once

#include <core/fs/File.hpp>
#include <core/fs/FileEntry.hpp>
#include <core/fs/FAT/FATHeaders.hpp>

class FATFileSystem;

class FATFile : public File {
public:
    FATFile();

    bool Open(FATFileSystem* fs, uint32_t firstCluster, const char* name, uint32_t size, bool isDirectory);
    bool OpenRootDirectory1216(FATFileSystem* fs, uint32_t rootDirLba, uint32_t rootDirSize);
    bool IsOpened() const { return m_Opened; }

    bool ReadFileEntry(FAT_DirectoryEntry* dirEntry);
    virtual FileEntry* ReadFileEntry() override;
    virtual void Release() override;

    virtual size_t Read(uint8_t* data, size_t count) override;
    virtual size_t Write(const uint8_t* data, size_t size) override;
    
    virtual bool Seek(int rel, SeekPos pos) override;
    
    virtual size_t Position() override { return m_Position; }
    virtual size_t Size() override { return m_Size; }

private:
    bool UpdateCurrentCluster();

    FATFileSystem* m_FS;
    uint8_t m_Buffer[SectorSize];
    bool m_Opened;
    bool m_IsRootDir;
    bool m_IsDirectory;
    uint32_t m_FirstCluster;
    uint32_t m_CurrentCluster;
    uint32_t m_CurrentClusterIdx;
    uint32_t m_CurrentSectorInCluster;
    uint32_t m_Position;
    uint32_t m_Size;
};
