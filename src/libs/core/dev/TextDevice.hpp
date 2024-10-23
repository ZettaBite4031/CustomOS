#pragma once
#include "CharacterDevice.hpp"
#include <core/cpp/TypeTraits.hpp>

#include <stdarg.h>
#include <stdbool.h>

class TextDevice {
public:
    TextDevice(CharacterDevice* dev);

    bool Write(char c);
    bool Write(const char* str);
    bool VWriteF(const char* fmt, va_list args);
    bool WriteF(const char* fmt, ...);
    bool WriteBuffer(const char* msg, const void* buffer, size_t size);

    template<typename T>
    bool Write(T num, int base = 10);

private:
    const char m_HexChars[16] = { 
            '0', '1', '2', '3', 
            '4', '5', '6', '7', 
            '8', '9', 'A', 'B', 
            'C', 'D', 'E', 'F' 
        };

    CharacterDevice* m_Device;
};
