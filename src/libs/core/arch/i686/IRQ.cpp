#include "IRQ.hpp"
#include "PIC.hpp"

#include <core/Debug.hpp>

#include "IO.hpp"

constexpr uint8_t CppPICRemapOffset{ 0x20 };

IRQ::IRQHandler g_CppIRQHandlers[16]{};
static PICDriver* g_CppIrqDriver{ nullptr };

void MainIRQHandler(ISR::Registers* regs) {
    int irq = regs->interrupt - CppPICRemapOffset;
    
    if (g_CppIRQHandlers[irq])
        g_CppIRQHandlers[irq](regs);
    else {
        Debug::Error("IRQ", "Unhandled IRQ %d!!", irq);
    }

    g_CppIrqDriver->SendEOI(irq);
}

void IRQ::Init() {
    Debug::Info("IRQ", "Beginning IRQ initialization");
    PICDriver* drivers[] {
        const_cast<i8259Driver*>(i8259Driver::GetDriver()),
    };

    for (int i = 0; i < _countof(drivers); i++) {
        if (drivers[i]->Probe()) {
            g_CppIrqDriver = drivers[i];
        }
    }

    if (!g_CppIrqDriver) {
        Debug::Error("IRQ", "Failed to find suitable PIC!");
        return;
    }

    Debug::Info("IRQ", "Suitable PIC Found: %s", g_CppIrqDriver->Name());
    g_CppIrqDriver->Initialize(CppPICRemapOffset, CppPICRemapOffset + 8, false);

    for (int i = 0; i < 16; i++)
        ISR::RegisterHandler(CppPICRemapOffset + i, MainIRQHandler);

    arch::i686::EnableInterrupts();
    Debug::Info("IRQ", "IRQ initialization successful");
}

void IRQ::RegisterHandler(int irq, IRQHandler handler) {
    g_CppIRQHandlers[irq] = handler;
}