#pragma once

#include <stdint.h>
#include <stddef.h>

#include "ISR.hpp"

namespace IRQ {
    using IRQHandler = void(*)(ISR::Registers*, void*);

    void Init();
    void RegisterHandler(int irq, IRQHandler handler, void* data = nullptr);
}