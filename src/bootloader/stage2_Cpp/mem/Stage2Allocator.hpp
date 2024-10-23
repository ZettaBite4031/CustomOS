#pragma once

#include <core/mem/Allocator.hpp>

#include <stdint.h>

class Stage2Allocator : public Allocator {
public:
    Stage2Allocator(void* base, size_t limit);

    virtual void* Allocate(size_t size) override;
    virtual void Free(void* addr) override;

private:
    uint8_t* m_Base;
    size_t m_Allocated;
    size_t m_Limit;
};