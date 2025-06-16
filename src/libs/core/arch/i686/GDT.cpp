#include "GDT.hpp"

#include <core/Debug.hpp>

GDT::GDTEntry g_CppGDT[3]{};
GDT::GDTDesc g_CppGDTDesc{};

#define GDT_LIMIT_LOW(limit)                (uint8_t)(limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (uint8_t)(base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               (uint8_t)((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (uint8_t)(((limit >> 16) & 0x0F) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 (uint8_t)((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) { \
    GDT_LIMIT_LOW(limit),                       \
    GDT_BASE_LOW(base),                         \
    GDT_BASE_MIDDLE(base),                      \
    (uint8_t)access,                            \
    GDT_FLAGS_LIMIT_HI(limit, flags),           \
    GDT_BASE_HIGH(base)                         \
}

enum Access : uint8_t {
    CodeReadable = 0x02,
    DataWritable = 0x02,

    CodeConforming = 0x04,
    DataDirectionNormal = 0x00,
    DataDirectionDown = 0x04,

    DataSegment = 0x10,
    CodeSegment = 0x18,

    DescriptorTSS = 0x00,

    Ring0 = 0x00,
    Ring1 = 0x20,
    Ring2 = 0x40,
    Ring3 = 0x60,

    Present = 0x80,
};

enum Flags : uint8_t {
    Bits64 = 0x20,
    Bits32 = 0x40,
    Bits16 = 0x00,

    Granularity1B = 0x00,
    Granularity4K = 0x80,
};

void GDT::Default() {
    g_CppGDT[0] = GDT_ENTRY(0, 0, Ring0, Bits16);
    g_CppGDT[1] = GDT_ENTRY(0, 0xFFFF, 
                Present | Ring0 | Access::CodeSegment | CodeReadable,
                Bits32 | Granularity4K);
    g_CppGDT[2] = GDT_ENTRY(0, 0xFFFF,
                Present | Ring0 | Access::DataSegment | DataWritable,
                Bits32 | Granularity4K);

    g_CppGDTDesc = { sizeof(g_CppGDT) - 1, &g_CppGDT[0] };
}

void GDT::Load() {
    arch::i686::LoadGDT(&g_CppGDTDesc, GDT::CodeSegment, 0x10);
    Debug::Info("GDT", "GDT Loaded successfully!");
}

void GDT::LoadDefaults() {
    Default();
    Load();
}