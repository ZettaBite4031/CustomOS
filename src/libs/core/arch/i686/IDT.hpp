#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/ZosDefs.hpp>

namespace IDT{
    struct IDTEntry {
        uint16_t BaseLow;
        uint16_t SegmentSelector;
        uint8_t Reserved;
        uint8_t Flags;
        uint16_t BaseHigh;
    } PACKED;

    struct IDTDesc {
        uint16_t Limit;
        IDTEntry* IDT;
    } PACKED;

    EXPORT void SetGate(int interrupt, void* base, uint16_t segmentDesc, uint8_t flags);
    void EnableGate(int interrupt);
    void DisableGate(int interrupt);
    void Load();

    enum IDT_FLAGS {
        FLAG_GATE_TASK              = 0x05,
        FLAG_GATE_16BIT_INT         = 0x06,
        FLAG_GATE_16BIT_TRAP        = 0x07,
        FLAG_GATE_32BIT_INT         = 0x0E,
        FLAG_GATE_32BIT_TRAP        = 0x0F,

        FLAG_RING0                  = (0 << 5),
        FLAG_RING1                  = (1 << 5),
        FLAG_RING2                  = (2 << 5),
        FLAG_RING3                  = (3 << 5),

        FLAG_PRESENT                = 0x80
        
    };

}