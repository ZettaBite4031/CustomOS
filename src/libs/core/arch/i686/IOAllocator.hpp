#pragma once

#include <stdint.h>

#include <core/std/vector.hpp>
#include <core/std/concepts.hpp>


typedef uint16_t IOOffset;

class IOAllocator;

class IORange {
public:
    IORange() : m_Address{0}, m_Length{0}, m_IsSharable{false} {}
    IORange(uint16_t addr, uint16_t len, bool share) : m_Address{addr}, m_Length{len}, m_IsSharable{share} {}

    template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
    void write(IOOffset offset, T& val);
    template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
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