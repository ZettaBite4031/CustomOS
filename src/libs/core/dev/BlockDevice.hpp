#pragma once

#include <stddef.h>
#include <stdint.h>
#include "CharacterDevice.hpp"

enum class SeekPos { 
    Set,
    Current,
    End,
};

class BlockDevice : public CharacterDevice {
public:
    virtual bool Seek(int rel, SeekPos pos) = 0;
    virtual size_t Position() = 0;
    virtual size_t Size() = 0;
};
