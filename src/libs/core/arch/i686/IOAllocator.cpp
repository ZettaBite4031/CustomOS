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
    return offset + bytes <= m_Length;
}