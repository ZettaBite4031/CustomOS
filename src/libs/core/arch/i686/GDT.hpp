#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/std/vector.hpp>
#include <core/arch/i686/IO.hpp>

namespace GDT {
    static constexpr uint16_t CodeSegment{ 0x08 };

    void Default();
    void Load();
    void LoadDefaults();
};