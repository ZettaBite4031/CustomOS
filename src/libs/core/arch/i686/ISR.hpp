#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/ZosDefs.hpp>

namespace ISR {
    struct Registers {
        uint32_t ds;
        uint32_t edi, esi, ebp, kernel_esp, ebx, edx, ecx, eax;
        uint32_t interrupt, error;
        uint32_t eip, cs, eflags, esp, ss;
    } PACKED;

    using ISRHandler = void(*)(Registers* regs);

    void Init();
    void RegisterHandler(int interrupt, ISRHandler handler);
}