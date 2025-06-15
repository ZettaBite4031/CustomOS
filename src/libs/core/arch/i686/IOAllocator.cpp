#include "IOAllocator.hpp"

#include <core/Debug.hpp>
#include <core/std/concepts.hpp>

IORange IOAllocator::RequestIORange(uint16_t address, uint16_t length, bool share) {
    IORange new_range{ address, length, share };

    size_t index = m_AllocatedRanges.find([new_range](IORange x){
        return new_range.m_Address == x.m_Address && new_range.m_Length == x.m_Length;
    });

    if (index != m_AllocatedRanges.size()) {
        if (!m_AllocatedRanges[index].m_IsSharable){
            Debug::Critical("IOAllocator", "Range %s-%s already allocated and is not sharable!", address, address + length);
            return {};
        }

        return m_AllocatedRanges[index];
    }

    m_AllocatedRanges.push_back(new_range);
    return new_range;
}

void IOAllocator::FreeIORange(IORange& range) {
    size_t index = m_AllocatedRanges.find([range](IORange x){
        return range.m_Address == x.m_Address && range.m_Length == x.m_Length;
    });
    
    if (index == m_AllocatedRanges.size()) {
        Debug::Error("IOAllocator", "Attempted to free unallocated range");
        return;
    }

    m_AllocatedRanges.erase(index);
}

bool IORange::VerifyOffset(IOOffset offset, uint32_t bytes) {
    return offset + bytes > m_Length;
}


template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
void IORange::write(IOOffset offset, T& val) {
    static_assert(std::is_unsigned<T>::value, "Only unsigned integer types allowed!");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Only 8, 16, or 32-bit values are supported by IO Ports!");

    if (!VerifyOffset(offset, sizeof(T))) {
        Debug::Critical("IORange::Write", "Offset + Sizeof(T) > IORange::Length (%s + %d > %s)", offset, sizeof(T), m_Length);
        return;
    }

    uint16_t port = m_Address + offset;

    if constexpr (sizeof(T) == 1) {
        uint8_t v = static_cast<uint8_t>(val);
        asm volatile("outb %0, %1" : : "a"(v), "d"(port));
    } else if constexpr (sizeof(T) == 2) {
        uint16_t v = static_cast<uint16_t>(val);
        asm volatile("outw %0, %1" : : "a"(v), "d"(port));
    } else if constexpr (sizeof(T) == 4) {
        uint32_t v = static_cast<uint32_t>(val);
        asm volatile("outl %0, %1" : : "a"(v), "d"(port));
    } 
    
    Debug::Critical("IORange::Write", "IO Ports do not support values larger than 32-bits!");
}

template<typename T, typename = std::enable_if_t<std::is_unsigned_v<T>>>
T IORange::read(IOOffset offset) {
    static_assert(std::is_unsigned<T>::value, "Only unsigned integer types allowed!");
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4, "Only 8, 16, or 32-bit values are supported by IO Ports!");
    
    if (!VerifyOffset(offset, sizeof(T))) {
        Debug::Critical("IORange::Read", "Offset + Sizeof(T) > IORange::Length (%s + %d > %s)", offset, sizeof(T), m_Length);
        return static_cast<T>(-1);
    }

    uint16_t port = m_Address + offset;

    if constexpr (sizeof(T) == 1) {
        uint8_t result;
        asm volatile("inb %1, %0" : "=a"(result) : "d"(port));
        return static_cast<T>(result);
    } else if constexpr (sizeof(T) == 2) {
        uint16_t result;
        asm volatile("inw %1, %0" : "=a"(result) : "d"(port));
        return static_cast<T>(result);
    } else if constexpr (sizeof(T) == 4) {
        uint32_t result;
        asm volatile("inl %1, %0" : "=a"(result) : "d"(port));
        return static_cast<T>(result);
    } 

    Debug::Critical("IORange::Write", "IO Ports do not support values larger than 32-bits!");
    return static_cast<T>(-1);
}