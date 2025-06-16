#include "ISR.hpp"

#include <core/Debug.hpp>
#include "IO.hpp"

#include "IDT.hpp"

ISR::ISRHandler g_CppISRHandlers[256]{};

static const char* const g_Exceptions[] = {
    "Divide by Zero",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound range exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault", "",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "", "", "", "", "", "",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception", ""
};

EXPORT void ASMCALL ISRSHandler(ISR::Registers* regs) {
    if (g_CppISRHandlers[regs->interrupt]) {
        g_CppISRHandlers[regs->interrupt](regs);
    } else if (regs->interrupt >= 32) Debug::Critical("ISR", "Unhandled Interrupt %d!!", regs->interrupt);
    else {
        Debug::Critical("ISR", "Unhandled Exception!! %d: %s!", regs->interrupt, g_Exceptions[regs->interrupt]);
        Debug::Critical("ISR", "Registers:");
        Debug::Critical("ISR", "EAX = 0x%x  |  EBX = 0x%x  |  ECX = 0x%x  |  EDX = 0x%x", regs->eax, regs->ebx, regs->ecx, regs->edx);
        Debug::Critical("ISR", "ESI = 0x%x  |  EDI = 0x%x  |  ESP = 0x%x  |  EBP = 0x%x", regs->esi, regs->edi, regs->esp, regs->ebp);
        Debug::Critical("ISR", "EIP = 0x%x  |   SS = 0x%x  |   CS = 0x%x  |   DS = 0x%x", regs->eip, regs->ss, regs->cs, regs->ds);
        Debug::Critical("ISR", "EFLAGS = 0x%x", regs->eflags);
        Debug::Critical("ISR", "!KERNEL PANIC!");
        arch::i686::PANIC();
    }
} 

EXPORT void ISR_Initialize();

void ISR::Init() {
    ISR_Initialize();
    for (int i = 0; i < 256; i++)
        IDT::EnableGate(i);
}

void ISR::RegisterHandler(int interrupt, ISRHandler handler) {
    g_CppISRHandlers[interrupt] = handler;
    IDT::EnableGate(interrupt);
}