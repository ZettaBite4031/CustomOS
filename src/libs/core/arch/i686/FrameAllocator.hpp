#pragma once

#include <stddef.h>
#include <stdint.h>

class FrameAllocator {
public:
    static void Init(uintptr_t mmap_start, size_t mmap_size);

    static uintptr_t Allocate();
    static uintptr_t AllocateContiguous(size_t num_frames);

    static void Free(uintptr_t phys_addr);

private:
    static constexpr size_t FRAME_SIZE = 4096;

    static uint8_t* bitmap;
    static size_t bitmap_size_bytes;

    static uintptr_t phys_start;
    static size_t total_frames;

    static bool IsFrameFree(size_t frame_idx);
    static void SetFrameUsed(size_t frame_idx, bool used);
};