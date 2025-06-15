#include "irq.h"
#include "pic.h"
#include "i8259.h"
#include "io.h"

#include <stddef.h>
#include <debug.h>
#include <stdio.h>
#include <zosdefs.h>

#define PIC_REMAP_OFFSET        0x20

IRQHandler g_IRQHandlers[16];
static const PIC_Driver* g_Driver = NULL;

void i686_IRQHandler(Registers* regs) {
    int irq = regs->interrupt - PIC_REMAP_OFFSET;
    
    if (g_IRQHandlers[irq] != NULL)
        g_IRQHandlers[irq](regs);
    else printf("Unhandled IRQ %d...\n", irq);
    
    g_Driver->SendEOI(irq);
}

void i686_IRQ_Initialize() {
    const PIC_Driver* drivers[] = {
        i8259_GetDriver(),
    };

    for (int i = 0; i < _countof(drivers); i++){
        if (drivers[i]->Probe()) {
            g_Driver = drivers[i];
        }
    }
    
    if (g_Driver == NULL) {
        LogError("IRQ", "Unable to find suitable PIC!");
        printf("WARNING: Unable to find suitable PIC!\n");
        return;
    }

    LogInfo("IRQ", "Suitable PIC Found! %s", g_Driver->Name);
    g_Driver->Initialize(PIC_REMAP_OFFSET, PIC_REMAP_OFFSET + 8, false);

    // register ISR handlers for each 16 irq lines
    for (int i = 0; i < 16; i++) 
        i686_ISR_RegisterHandler(PIC_REMAP_OFFSET + i, i686_IRQHandler);

    // enable interrupts
    i686_EnableInterrupts();

    //g_Driver->Unmask(0);
    //g_Driver->Unmask(1);
}

void i686_IRQ_RegisterHandler(int irq, IRQHandler handler){ 
    g_IRQHandlers[irq] = handler;
}
