#pragma once

#include <core/fs/File.hpp>


enum class FileType {
    File, 
    Directory,
};

enum class FileOpenMode {
    Read,
    Write,
    Append
};

class FileEntry {
public:
    virtual File* Open(FileOpenMode mode) = 0;
    virtual void Release() = 0;

    virtual const char* Name() = 0;
    virtual const FileType Type() = 0;
};
