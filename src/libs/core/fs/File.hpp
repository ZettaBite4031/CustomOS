#pragma once

#include <core/dev/BlockDevice.hpp>

class FileEntry;

class File : public BlockDevice {
public:
    virtual FileEntry* ReadFileEntry() = 0;
    virtual void Release() = 0;
    virtual bool Resize(size_t size) = 0;
    virtual bool EraseContents() = 0;
};
