#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "memory.h"
#include "debug.h"
#include "zosdefs.h"
#include "stdio.h"

#define ASSERT(cond, ...) do { if (!(cond)) { LogError( __VA_ARGS__); return false; } } while (0);

void dump_heap();

bool test_allocator() {
    // malloc test 
    void* a = malloc(32);
    ASSERT(a != NULL, "Allocator Test", "malloc(32) failed!");

    ((uint8_t*)a)[0] = 0xAA;
    ((uint8_t*)a)[31] = 0x55;
    ASSERT(((uint8_t*)a)[0] == 0xAA, "Allocator Test", "write check failed!");
    ASSERT(((uint8_t*)a)[31] == 0x55, "Allocator Test", "write check failed!");

    // calloc test
    void* b = calloc(64);
    ASSERT(b != NULL, "Allocator Test", "calloc(64)");
    for (uint32_t i = 0; i < 64; i++) ASSERT(((uint8_t*)b)[i] == 0x0, "Allocator Test", "calloc did not zero memory! ");

    // realloc test (grow)
    void* c = realloc(a, 64);
    ASSERT(c != NULL, "Allocator Test", "realloc(32 -> 64) failed");
    ASSERT(((uint8_t*)c)[0] == 0xAA, "Allocator Test", "realloc data not preserved");
    ASSERT(((uint8_t*)c)[31] == 0x55, "Allocator Test", "realloc data not preserved");

    // realloc test (shrink)
    void* d = realloc(c, 16);
    ASSERT(d != NULL, "Allocator Test", "realloc(64 -> 16) failed");
    ASSERT(((uint8_t*)d)[0] == 0xAA, "Allocator Test", "realloc data not preserved!");

    // recalloc test (grow with zeroing)
    void* e = recalloc(d, 32);
    ASSERT(e != NULL, "Allocator Test", "recalloc(16 -> 32) failed");
    for (uint32_t i = 16; i < 32; i++) ASSERT(((uint8_t*)e)[i] == 0, "Allocator Test", "recalloc didn't zero new bytes");
    LogInfo("Allocator Test", "All allocator tests passed!")

    // free remaining pointers
    free(b);
    free(e);

    dump_heap();

    return true;
}