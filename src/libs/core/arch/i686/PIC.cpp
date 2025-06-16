#include "PIC.hpp"
#include "IO.hpp"

#include <core/cpp/String.hpp>

using namespace arch::i686;

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

bool i8259Driver::Probe() {
    this->Disable();
    this->SetMask(0x4031);
    return (this->GetMask() == 0x4031);
}

void i8259Driver::Initialize(uint8_t offset1, uint8_t offset2, bool autoEOI) {
    m_Name = strdup("8259 PIC");;
    this->SetMask(0xFFFF);

    // Initialization Control Word 1
    OutPortB(PIC1CommandPort, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    IOWait();
    OutPortB(PIC2CommandPort, PIC_ICW1_ICW4 | PIC_ICW1_INITIALIZE);
    IOWait();

    // Initialization Control Word 2
    OutPortB(PIC1DataPort, offset1);
    IOWait();
    OutPortB(PIC2DataPort, offset2);
    IOWait;

    // Initialization Control Word 3
    OutPortB(PIC1DataPort, 0x04);
    IOWait();
    OutPortB(PIC2DataPort, 0x02);
    IOWait();

    // Initialization Control Word 4
    m_AutoEOI = autoEOI;
    uint8_t icw4 = PIC_ICW4_8086;
    if (m_AutoEOI)
        icw4 |= PIC_ICW4_AUTO_EOI;
    OutPortB(PIC1DataPort, icw4);
    IOWait();
    OutPortB(PIC2DataPort, icw4);
    IOWait();

    this->SetMask(0xFFFF);
}

void i8259Driver::Disable() {
    this->SetMask(0xFFFF);
}

void i8259Driver::SendEOI(int irq) {
    if (irq >= 8)
        OutPortB(PIC2CommandPort, PIC_CMD_END_OF_INTERRUPT);
    OutPortB(PIC1CommandPort, PIC_CMD_END_OF_INTERRUPT);
}

void i8259Driver::Mask(int irq) {
    this->SetMask(m_PICMask | (1 << irq));
}

void i8259Driver::Unmask(int irq) {
    this->SetMask(m_PICMask & ~(1 << irq));
}


uint16_t i8259Driver::ReadInServiceRegister() {
    OutPortB(PIC1CommandPort, PIC_CMD_READ_ISR);
    OutPortB(PIC2CommandPort, PIC_CMD_READ_ISR);
    return ((uint16_t)InPortB(PIC2CommandPort)) | (((uint16_t)InPortB(PIC2CommandPort)) << 8);
}

uint16_t i8259Driver::ReadIRQRequestRegister() {
    OutPortB(PIC1CommandPort, PIC_CMD_READ_IRR);
    OutPortB(PIC2CommandPort, PIC_CMD_READ_IRR);
    return ((uint16_t)InPortB(PIC2CommandPort)) | (((uint16_t)InPortB(PIC2CommandPort)) << 8);
}

uint16_t i8259Driver::GetMask() {
    return ((uint16_t)InPortB(PIC1DataPort)) | (((uint16_t)InPortB(PIC2DataPort)) << 8);
}

void i8259Driver::SetMask(int mask) {
    m_PICMask = mask;
    OutPortB(PIC1DataPort, m_PICMask & 0xFF);
    IOWait();
    OutPortB(PIC2DataPort, m_PICMask >> 8);
    IOWait();
}

