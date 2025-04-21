#pragma once
#include "stdint.h"
#include <boot/bootparams.h>
#include <stdbool.h>
#include "zosdefs.h"

#define ALIGN_UP(ptr, alignment) (((ptr) + ((alignment) - 1)) & ~((alignment) - 1))

void* EXTERN memcpy(void* dst, const void* src, uint16_t num);
void* EXTERN memset(void* ptr, int value, uint16_t num);
int EXTERN memcmp(const void* ptr1, const void* ptr2, uint16_t num);

uint32_t get_alignment(void* ptr);

bool Mem_Init(MemoryInfo* mem_info);
void* malloc(uint32_t size);
void* calloc(uint32_t size);
void free(void* ptr);
void* realloc(void* ptr, uint32_t new_size);
void* recalloc(void* ptr, uint32_t new_size);

void* malloc_aligned(uint32_t size, uint32_t alignment);
void free_aligned(void* ptr);
