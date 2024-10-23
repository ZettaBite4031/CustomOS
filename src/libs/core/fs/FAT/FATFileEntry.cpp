#include "FATFileEntry.hpp"

#include <fs/FATFileSystem.hpp>

#include <core/Debug.hpp>

FATFileEntry::FATFileEntry()
    : m_FS(), m_DirEntry() {}

void FATFileEntry::Initialize(FATFileSystem* fs, const FAT_DirectoryEntry& dirEntry) {
    m_FS = fs;
    m_DirEntry = dirEntry;
}

void FATFileEntry::Release() {
    m_FS->ReleaseFileEntry(this);
}

File* FATFileEntry::Open(FileOpenMode mode) {
    FATFile* file = m_FS->AllocateFile();
    if (!file) {
        Debug::Error("FatFileEntry", "Could not allocate a new file!");
        return nullptr;
    }

    uint32_t firstCluster = m_DirEntry.FirstClusterLow + ((uint32_t)m_DirEntry.FirstClusterHigh << 16);
    if (!file->Open(m_FS, firstCluster, Name(), m_DirEntry.Size, m_DirEntry.Attributes & FAT_ATTRIBUTE_DIRECTORY)) {
        Debug::Error("FatFileEntry", "Failed to open file!");
        m_FS->ReleaseFile(file);
        return nullptr;
    }

    return file;
}
