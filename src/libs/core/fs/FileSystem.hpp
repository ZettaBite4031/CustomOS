#pragma once

#include <core/dev/BlockDevice.hpp>

#include "FileEntry.hpp"
#include "File.hpp"

constexpr size_t MaxPathSize = 256;

class FileSystem {
public:
    virtual bool Initialize(BlockDevice* device) = 0;
    virtual File* RootDirectory() = 0;
    
    virtual File* Open(const char* path, FileOpenMode openMode);

private:
    virtual FileEntry* FindFile(File* parentDir, const char* name);
};
