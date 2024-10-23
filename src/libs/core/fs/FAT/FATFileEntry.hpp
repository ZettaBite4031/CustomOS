#pragma once

#include <core/fs/FileEntry.hpp>
#include <core/fs/FAT/FATHeaders.hpp>

class FATFileSystem;

class FATFileEntry : public FileEntry {
public:
    FATFileEntry();
    void Initialize(FATFileSystem* fs, const FAT_DirectoryEntry& dirEntry);
    virtual File* Open(FileOpenMode mode) override;
    virtual void Release() override;

    virtual const char* Name() override { return reinterpret_cast<const char*>(m_DirEntry.Name); }
    virtual const FileType Type() override { return (m_DirEntry.Attributes & FAT_ATTRIBUTE_DIRECTORY) ? FileType::Directory : FileType::File; }
    
private:
    FATFileSystem* m_FS;
    FAT_DirectoryEntry m_DirEntry;
};
