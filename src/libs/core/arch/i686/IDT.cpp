#include "IDT.hpp"

#include <core/cpp/String.hpp>

#include "IO.hpp"


IDT::IDTEntry g_CppIDT[256];
IDT::IDTDesc g_CppIDTDesc = { sizeof(g_CppIDT) - 1, &g_CppIDT[0] };

EXPORT void IDT::SetGate(int interrupt, void* base, uint16_t segmentDesc, uint8_t flags) {
    g_CppIDT[interrupt].BaseLow = ((uint32_t)base) & 0xFFFF;
    g_CppIDT[interrupt].SegmentSelector = segmentDesc;
    g_CppIDT[interrupt].Reserved = 0x00;
    g_CppIDT[interrupt].Flags = flags;
    g_CppIDT[interrupt].BaseHigh = ((uint32_t)(base) >> 16) & 0xFFFF;
}

void IDT::EnableGate(int interrupt) {
    FLAG_SET(g_CppIDT[interrupt].Flags, FLAG_PRESENT);
}

void IDT::DisableGate(int interrupt) {
    FLAG_UNSET(g_CppIDT[interrupt].Flags, FLAG_PRESENT);
}

void IDT::Load() {
    arch::i686::LoadIDT(&g_CppIDTDesc);
}
