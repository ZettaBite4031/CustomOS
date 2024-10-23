#include "isr.h"
#include "gdt.h"
#include "idt.h"
#include "io.h"

#include <stdio.h>
#include <debug.h>
#include <stddef.h>

#include <zosdefs.h>

ISRHandler g_ISRHandlers[256];

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

void i686_ISR_InitializeGates();

void i686_ISR_Initialize() {
    i686_ISR_InitializeGates();
    for (int i = 0; i < 256; i++) 
        i686_IDT_EnableGate(i);
}

void GLOBAL i686_ISR_Handler(Registers* regs) {
    if (g_ISRHandlers[regs->interrupt] != NULL)
        g_ISRHandlers[regs->interrupt](regs);
    else if (regs->interrupt >= 32) printf("Unhandled interrupt %d!!\n", regs->interrupt);
    else {
        LogCritical("ISR", "Unhandled Exception!! %d: %s | Registers printed to kernel!", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("Unhandled Exception!! %d: %s\n", regs->interrupt, g_Exceptions[regs->interrupt]);
        printf("Registers:\n");
        printf("EAX = 0x%x  |  EBX = 0x%x  |  ECX = 0x%x  |  EDX = 0x%x\n", regs->eax, regs->ebx, regs->ecx, regs->edx);
        printf("ESI = 0x%x  |  EDI = 0x%x  |  ESP = 0x%x  |  EBP = 0x%x\n", regs->esi, regs->edi, regs->esp, regs->ebp);
        printf("EIP = 0x%x  |   SS = 0x%x  |   CS = 0x%x  |   DS = 0x%x\n", regs->eip, regs->ss, regs->cs, regs->ds);
        printf("EFLAGS = 0x%x\n", regs->eflags);
        LogCritical("ISR", "!KERNEL PANIC!");
        printf("\n!KERNEL PANIC!\n\n");
        i686_PANIC();
    }
}

void i686_ISR_RegisterHandler(int interrupt, ISRHandler handler) {
    g_ISRHandlers[interrupt] = handler;
    i686_IDT_EnableGate(interrupt);
}