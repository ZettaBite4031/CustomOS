#include "idt.hpp"
#include <util/binary.h>
#include <zosdefs.h>

typedef struct {
    uint16_t BaseLow;
    uint16_t SegmentSelector;
    uint8_t Reserved;
    uint8_t Flags;
    uint16_t BaseHigh;
} PACKED IDTEntry;

typedef struct {
    uint16_t Limit;
    IDTEntry* IDT;
} PACKED IDTDescriptor;

IDTEntry g_IDT[256];

IDTDescriptor g_IDTDesc = { sizeof(g_IDT) - 1, g_IDT };

extern "C" void EXTERN i686_IDT_Load(IDTDescriptor* desc);

extern "C" void i686_IDT_SetGate(int interrupt, void* base, uint16_t segemntDesc, uint8_t flags) {
    g_IDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_IDT[interrupt].SegmentSelector = segemntDesc;
    g_IDT[interrupt].Reserved = 0;
    g_IDT[interrupt].Flags = flags;
    g_IDT[interrupt].BaseHigh = ((uint32_t)(base) >> 16) & 0xFFFF;
}

void i686_IDT_EnableGate(int interrupt) {
    FLAG_SET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_DisableGate(int interrupt) {
    FLAG_UNSET(g_IDT[interrupt].Flags, IDT_FLAG_PRESENT);
}

void i686_IDT_Initialize() {
    i686_IDT_Load(&g_IDTDesc);
}