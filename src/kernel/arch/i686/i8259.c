#include "i8259.h"
#include "io.h"
#include <stdio.h>

#define PIC1_COMMAND_PORT       0x20
#define PIC1_DATA_PORT          0x21
#define PIC2_COMMAND_PORT       0xA0
#define PIC2_DATA_PORT          0xA1

/*  Initialization Control Word 1 (ICW1)
    ------------------------------------
    0    IC4     If set, PIC expects ICW4 during initialization
    1    SINGLE  If set, only 1 PIC in the system. If unset, cascade mode with slave PICs
    2    ADI     Ignored on x86, set to 0
    3    MODE    Trigger mode. If set, level triggered; unset, edge triggered
    4    INIT    Set to 1 to initialize PIC
    5-7          Ignored on x86, set to 0
*/
enum {
    PIC_ICW1_ICW4           = 0x01,
    PIC_ICW1_SINGLE         = 0x02,
    PIC_ICW1_ADDR_INTERVAL  = 0x04,
    PIC_ICW1_TRIGGER_MODE   = 0x08,
    PIC_ICW1_INITIALIZE     = 0x10,
} PIC_ICW1;

/*  Initialization Control Word 4 (ICW4)
    ------------------------------------
    0   uPM     If set, PIC enters 80x86 mode; unset, enters MCS-80/86 mode
    1   AEOI    If set, performs automatic End of Interrupt (EOI)
    2   M/S     Buffered mode master/slave. Master if set, slave if unset. Only relevent when BUF is set
    3   BUF     Buffered mode enabled.
    4   SFNM    Special Fully Nested Mode. Used in systems with large numbers of cascaded controllers
    5-7 Reserved
*/
enum {
    PIC_ICW4_8086           = 0x01,
    PIC_ICW4_AUTO_EOI       = 0x02,
    PIC_ICW4_BUFFER_MASTER  = 0x04,
    PIC_ICW4_BUFFER_SLAVE   = 0x00,
    PIC_ICW4_BUFFERED       = 0x08,
    PIC_ICW4_SFNM           = 0x10,
} PIC_ICW4;

enum {
    PIC_CMD_END_OF_INTERRUPT    = 0x20,
    PIC_CMD_READ_IRR            = 0x0A,
    PIC_CMD_READ_ISR            = 0x0B,
} PIC_CMD;

static uint16_t g_PicMask = 0xFFFF;
static bool g_AutoEOI = false;

void i8259_SetMask(uint16_t newMask) {
    g_PicMask = newMask; // mask all
    i686_OutB(PIC1_DATA_PORT, g_PicMask & 0xFF);
    i686_IOWait();
    i686_OutB(PIC2_DATA_PORT, g_PicMask >> 8);
    i686_IOWait();
}

void i8259_Configure(uint8_t offsetPic1, uint8_t offsetPic2, bool autoEOI) {
    // Mask everything
    i8259_SetMask(0xFFFF);

    // Initialization Control Word 1
    i686_OutB(PIC1_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_IOWait();
    i686_OutB(PIC2_COMMAND_PORT, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    i686_IOWait();

    // Initialization Control Word 2 - offsets
    i686_OutB(PIC1_DATA_PORT, offsetPic1);
    i686_IOWait();
    i686_OutB(PIC2_DATA_PORT, offsetPic2);
    i686_IOWait();

    // Initialization Control Word 3 
    i686_OutB(PIC1_DATA_PORT, 0x4);
    i686_IOWait();
    i686_OutB(PIC2_DATA_PORT, 0x2);
    i686_IOWait();

    // Initialization Control Word 4
    g_AutoEOI = autoEOI;
    uint8_t icw4 = PIC_ICW4_8086;
    if (g_AutoEOI)
        icw4 |= PIC_ICW4_AUTO_EOI;
    
    i686_OutB(PIC1_DATA_PORT, icw4);
    i686_IOWait();
    i686_OutB(PIC2_DATA_PORT, icw4);
    i686_IOWait();

    // mask all interrupts
    i8259_SetMask(0xFFFF);
}

void i8259_Disable() {
    i8259_SetMask(0xFFFF);
}

uint16_t i8259_GetMask() {
    return((uint16_t)i686_InB(PIC1_DATA_PORT)) | (((uint16_t)i686_InB(PIC2_DATA_PORT)) << 8);
}

void i8259_Mask(int irq) {
    i8259_SetMask(g_PicMask |= (1 << irq));
}

void i8259_Unmask(int irq) {
    i8259_SetMask(g_PicMask & ~(1 << irq));
}

uint16_t i8259_ReadIRQRequestRegister() {
    i686_OutB(PIC1_COMMAND_PORT, PIC_CMD_READ_IRR);
    i686_OutB(PIC2_COMMAND_PORT, PIC_CMD_READ_IRR);
    return((uint16_t)i686_InB(PIC2_COMMAND_PORT)) | (((uint16_t)i686_InB(PIC1_COMMAND_PORT)) << 8);
}

uint16_t i8259_ReadInServiceRegister() {
    i686_OutB(PIC1_COMMAND_PORT, PIC_CMD_READ_ISR);
    i686_OutB(PIC2_COMMAND_PORT, PIC_CMD_READ_ISR);
    return((uint16_t)i686_InB(PIC2_COMMAND_PORT)) | (((uint16_t)i686_InB(PIC1_COMMAND_PORT)) << 8);
}

void i8259_SendEndOfInterrupt(int irq) {
    if (irq >= 8) 
        i686_OutB(PIC2_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
    i686_OutB(PIC1_COMMAND_PORT, PIC_CMD_END_OF_INTERRUPT);
}

bool i8259_Probe() {
    i8259_Disable();
    i8259_SetMask(0x4031);
    return (i8259_GetMask() == 0x4031);
}

static const PIC_Driver g_PICDriver = {
    .Name = "8259 PIC",
    .Probe = &i8259_Probe,
    .Initialize = &i8259_Configure,
    .Disable = &i8259_Disable,
    .SendEOI = &i8259_SendEndOfInterrupt,
    .Mask = &i8259_Mask,
    .Unmask = &i8259_Unmask,
};

const PIC_Driver* i8259_GetDriver() {
    return &g_PICDriver;
}