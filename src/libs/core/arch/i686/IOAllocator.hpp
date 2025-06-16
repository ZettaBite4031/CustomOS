#pragma once

#include <stdint.h>

#include <core/std/vector.hpp>
#include <core/std/concepts.hpp>

#include <core/Debug.hpp>

#include "IO.hpp"

typedef uint16_t IOOffset;

class IOAllocator;

class IORange {
public:
    IORange() : m_Address{0}, m_Length{0}, m_IsSharable{false} {}
    IORange(uint16_t addr, uint16_t len, bool share) : m_Address{addr}, m_Length{len}, m_IsSharable{share} {}

    template<typename T>
    void write(IOOffset offset, T val);
    template<typename T>
    T read(IOOffset offset);

private:
    bool VerifyOffset(IOOffset offset, uint32_t bytes);

    friend class IOAllocator;

    uint16_t m_Address;
    uint16_t m_Length;
    bool m_IsSharable;
};

class IOAllocator {
public:
    IOAllocator() = default;

    IORange RequestIORange(uint16_t address, uint16_t length, bool share);
    void FreeIORange(IORange& range);

private:
    IORange IsRangeAllocated(IORange range);

    std::vector<IORange> m_AllocatedRanges{};
};



template<typename T>
void IORange::write(IOOffset offset, T val) {
    static_assert(std::is_unsigned<T>::value, "Only unsigned integer types allowed!");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Only 8, 16, or 32-bit values are supported by IO Ports!");

    if (!VerifyOffset(offset, sizeof(T))) {
        Debug::Critical("IORange::Write", "Offset + Sizeof(T) > IORange::Length (%u + %u > %u)", offset, sizeof(T), m_Length);
        return;
    }

    uint16_t port = m_Address + offset;

    if constexpr (sizeof(T) == 1) {
        uint8_t v = static_cast<uint8_t>(val);
        arch::i686::OutPortB(port, v);
    } else if constexpr (sizeof(T) == 2) {
        uint16_t v = static_cast<uint16_t>(val);        
        arch::i686::OutPortW(port, v);
    } else if constexpr (sizeof(T) == 4) {
        uint32_t v = static_cast<uint32_t>(val);
        arch::i686::OutPortL(port, v);
    } else {
        Debug::Critical("IORange::Write", "IO Ports do not support values larger than 32-bits!");
    }
    
}

template<typename T>
T IORange::read(IOOffset offset) {
    static_assert(std::is_unsigned<T>::value, "Only unsigned integer types allowed!");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Only 8, 16, or 32-bit values are supported by IO Ports!");
    
    if (!VerifyOffset(offset, sizeof(T))) {
        Debug::Critical("IORange::Read", "Offset + Sizeof(T) > IORange::Length (%s + %d > %s)", offset, sizeof(T), m_Length);
        return static_cast<T>(-1);
    }

    uint16_t port = m_Address + offset;

    if constexpr (sizeof(T) == 1) {
        uint8_t result = arch::i686::InPortB(port);
        return static_cast<T>(result);
    } else if constexpr (sizeof(T) == 2) {
        uint16_t result = arch::i686::InPortW(port);
        return static_cast<T>(result);
    } else if constexpr (sizeof(T) == 4) {
        uint32_t result = arch::i686::InPortL(port);
        return static_cast<T>(result);
    } 

    Debug::Critical("IORange::Write", "IO Ports do not support values larger than 32-bits!");
    return static_cast<T>(-1);
}