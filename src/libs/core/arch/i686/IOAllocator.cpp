#include "IOAllocator.hpp"

#include <core/Debug.hpp>
#include <concepts>
#include <algorithm>

IORange IOAllocator::RequestIORange(uint16_t address, uint16_t length, bool share) {
    IORange new_range{ address, length, share };

    auto it = std::find_if(m_AllocatedRanges.begin(), m_AllocatedRanges.end(),
        [&](const IORange& x) {
            return x.m_Address == new_range.m_Address && x.m_Length == new_range.m_Length;
        });
    
    if (it != m_AllocatedRanges.end()) {
        if (!it->m_IsSharable) {
            Debug::Critical("IOAllocator", 
                            "Range %u-%u is already allocated and not sharable",
                            static_cast<unsigned>(address), static_cast<unsigned>(address + length));
            return {};
        }
        return *it;
    }

    m_AllocatedRanges.push_back(new_range);
    return new_range;
}

void IOAllocator::FreeIORange(IORange& range) {
    auto it = std::find_if(m_AllocatedRanges.begin(), m_AllocatedRanges.end(),
        [&](const IORange& x) {
            return x.m_Address == range.m_Address && x.m_Length == range.m_Length;
        });
    if (it == m_AllocatedRanges.end()) {
        Debug::Error("IOAllocator", "Attempted to free unallocated range");
        return;
    }

    m_AllocatedRanges.erase(it);
}

bool IORange::VerifyOffset(IOOffset offset, uint32_t bytes) {
    return static_cast<uint32_t>(offset) + bytes <= static_cast<uint32_t>(m_Length);
}