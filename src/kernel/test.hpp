#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <core/cpp/Memory.hpp>
#include "zosdefs.h"
#include "stdio.h"

#include <core/Debug.hpp>
#include <core/Assert.hpp>


void dump_heap();

bool test_allocator() {
    // malloc test 
    void* a = malloc(32);
    Assert(a != NULL);

    ((uint8_t*)a)[0] = 0xAA;
    ((uint8_t*)a)[31] = 0x55;
    Assert(((uint8_t*)a)[0] == 0xAA);
    Assert(((uint8_t*)a)[31] == 0x55);

    // calloc test
    void* b = calloc(64);
    Assert(b != NULL);
    for (uint32_t i = 0; i < 64; i++) Assert(((uint8_t*)b)[i] == 0x0);

    // realloc test (grow)
    void* c = realloc(a, 64);
    Assert(c != NULL);
    Assert(((uint8_t*)c)[0] == 0xAA);
    Assert(((uint8_t*)c)[31] == 0x55);

    // realloc test (shrink)
    void* d = realloc(c, 16);
    Assert(d != NULL);
    Assert(((uint8_t*)d)[0] == 0xAA);

    // recalloc test (grow with zeroing)
    void* e = recalloc(d, 32);
    Assert(e != NULL);
    for (uint32_t i = 16; i < 32; i++) Assert(((uint8_t*)e)[i] == 0);
    Debug::Info("Allocator Test", "All allocator tests passed!");

    // free remaining pointers
    free(b);
    free(e);

    dump_heap();

    return true;
}