#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/std/vector.hpp>
#include <core/arch/i686/IO.hpp>

namespace GDT {
    struct GDTEntry {
        uint16_t LimitLow;
        uint16_t BaseLow;
        uint8_t BaseMiddle;
        uint8_t Access;
        uint8_t FlagsLimitHigh;
        uint8_t BaseHigh;
    } PACKED;

    struct GDTDesc {
        uint16_t limit;
        GDTEntry* GDT;
    } PACKED;

    static constexpr uint16_t CodeSegment{ 0x08 };

    void Default();
    void Load();
    void LoadDefaults();
};