#include "NewDelete.hpp"
#include <stddef.h>

Allocator* g_CppAllocator;

void SetCppAllocator(Allocator* allocator) {
    g_CppAllocator = allocator;
}

void* operator new(size_t size) throw() {
    if (!g_CppAllocator) return nullptr;
    return g_CppAllocator->Allocate(size);
}

void* operator new[](size_t size) throw() {
    if (!g_CppAllocator) return nullptr;
    return g_CppAllocator->Allocate(size);
}

void operator delete(void* p) {
    if (!g_CppAllocator) return;
    g_CppAllocator->Free(p);
}

void operator delete[](void* p) {
    if (!g_CppAllocator) return;
    g_CppAllocator->Free(p);
}
