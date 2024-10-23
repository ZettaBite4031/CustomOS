#include "Stage2Allocator.hpp"

#include <core/Debug.hpp>

Stage2Allocator::Stage2Allocator(void* base, size_t limit) 
    : m_Base(reinterpret_cast<uint8_t*>(base)), m_Limit(limit) {}


void* Stage2Allocator::Allocate(size_t size) {
    if (m_Allocated + size >= m_Limit) {
        Debug::Error("Stage2Allocator", "Attempted to allocate more than the limit! Total Allocated: %lu, Bytes Requested: %lu, Limit: %lu", m_Allocated, size, m_Limit);
        return nullptr;
    }

    void* addr = m_Base + m_Allocated;
    m_Allocated += size;
    return addr;
}

void Stage2Allocator::Free(void* addr) {
    // nop
}
