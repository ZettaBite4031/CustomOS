#pragma once

#include <stddef.h>

#include <core/cpp/Memory.hpp>

template <typename T, size_t PoolSize>
class StaticObjectPool {
public:
    StaticObjectPool();
    
    T* Allocate();
    void Free(T* obj);

private:
    T m_Pool[PoolSize];
    bool m_ObjectMask[PoolSize];
    size_t m_PoolSize;
};

template <typename T, size_t PoolSize>
StaticObjectPool<T, PoolSize>::StaticObjectPool()
    : m_PoolSize(0), m_ObjectMask(), m_Pool() {
    Memory::Set(&m_ObjectMask[0], false, PoolSize);
}


template <typename T, size_t PoolSize>
T* StaticObjectPool<T, PoolSize>::Allocate() {
    if (m_PoolSize >= PoolSize) return nullptr;
    
    for (size_t i = 0; i < PoolSize; i++) {
        size_t idx = (i + m_PoolSize) % PoolSize;
        if (!m_ObjectMask[idx]) {
            m_ObjectMask[idx] = true;
            m_PoolSize++;
            return &m_Pool[idx];
        }
    }
    
    return nullptr;
}

template <typename T, size_t PoolSize>
void StaticObjectPool<T, PoolSize>::Free(T* obj) {
    if (obj < m_Pool || obj >= m_Pool + PoolSize || m_PoolSize < 1) return;

    size_t idx = obj - m_Pool;
    m_ObjectMask[idx] = false;
    m_PoolSize--;
}
