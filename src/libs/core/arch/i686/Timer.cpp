#include "Timer.hpp"

#include "PIC.hpp"
#include "IRQ.hpp"
#include "IO.hpp"

#include <core/Debug.hpp>

volatile uint64_t PITTicks{ 0 };
PICDriver* g_Driver{ nullptr };
uint32_t g_PITHZ{ 0 };

void PITHandler(ISR::Registers* regs) {
    PITTicks++;
    g_Driver->SendEOI(0);
}

void PIT::Init(uint32_t frequency) {
    g_Driver = const_cast<i8259Driver*>(i8259Driver::GetDriver());
    g_PITHZ = frequency;

    uint32_t divisor = 1193182 / frequency;

    arch::i686::OutPortB(0x43, 0x36);
    arch::i686::OutPortB(0x40, (uint8_t)(divisor & 0xFF));
    arch::i686::OutPortB(0x40, (uint8_t)((divisor >> 8) & 0xFF));

    IRQ::RegisterHandler(0, PITHandler);
    g_Driver->Unmask(0x0);
    Debug::Info("PIT", "PIT initialized. Frequency: %d", g_PITHZ);
}

void sleep(uint32_t ms) {
    uint32_t start = PITTicks;
    uint32_t target = (ms * g_PITHZ) / 1000;

    while ((PITTicks - start) < target) 
        __asm__ __volatile("sti; hlt; cli;");
}
