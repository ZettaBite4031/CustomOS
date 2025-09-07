#include "FrameAllocator.hpp"

#include <core/cpp/Memory.hpp>

uint8_t* FrameAllocator::bitmap = nullptr;
size_t FrameAllocator::bitmap_size_bytes = 0;
uintptr_t FrameAllocator::phys_start = 0;
size_t FrameAllocator::total_frames = 0;

void FrameAllocator::Init(uintptr_t mmap_start, size_t mmap_size) {
    phys_start = mmap_start;
    total_frames = mmap_size / FRAME_SIZE;

    bitmap_size_bytes = (total_frames + 7) / 8;

    static uint8_t static_bitmap[65536];
    bitmap = static_bitmap;

    Memory::Set(bitmap, 0x00, bitmap_size_bytes);
}

uintptr_t FrameAllocator::Allocate() {
    for (size_t i = 0; i < total_frames; i++) {
        if (IsFrameFree(i)) {
            SetFrameUsed(i, true);
            return phys_start + i * FRAME_SIZE;
        }
    }
    return 0;
}

uintptr_t FrameAllocator::AllocateContiguous(size_t num_frames) {
    if (num_frames == 0 || num_frames > total_frames) return 0;

    size_t consecutive = 0;
    size_t start_idx = 0;

    for (size_t i = 0; i < total_frames; i++) {
        if (IsFrameFree(i)) {
            if (consecutive == 0) start_idx = i;
            consecutive++;
            if (consecutive == num_frames) {
                for (size_t j = start_idx; j < start_idx + num_frames; j++) {
                    SetFrameUsed(j, true);
                }
                return phys_start + start_idx * FRAME_SIZE;
            } 
        }else {
            consecutive = 0;
        }
    }
    return 0;
}

void FrameAllocator::Free(uintptr_t phys_addr) {
    if (phys_addr < phys_start) return;

    size_t frame_idx = (phys_addr - phys_start) / FRAME_SIZE;
    if (frame_idx < total_frames) SetFrameUsed(frame_idx, false);
}

bool FrameAllocator::IsFrameFree(size_t frame_idx) {
    size_t byte_index = frame_idx / 8;
    size_t bit_index = frame_idx % 8;
    return (bitmap[byte_index] & (1 << bit_index)) == 0;
}

void FrameAllocator::SetFrameUsed(size_t frame_idx, bool used) {
    size_t byte_index = frame_idx / 8;
    size_t bit_index = frame_idx % 8; 
    if (used)
        bitmap[byte_index] |= (1 << bit_index);
    else
        bitmap[byte_index] &= ~(1 << bit_index);
}