#pragma once

#include <stdint.h>
#include <stddef.h>

class PICDriver {
public:
    virtual bool Probe() = 0;
    virtual void Initialize(uint8_t offset1, uint8_t offset2, bool autoEOI) = 0;
    virtual void Disable() = 0;
    virtual void SendEOI(int) = 0;
    virtual void Mask(int) = 0;
    virtual void Unmask(int) = 0;
    virtual const char* Name() = 0;
};

class i8259Driver : public PICDriver {
public:
    static const i8259Driver* GetDriver();

    virtual bool Probe() override;
    virtual void Initialize(uint8_t offset1, uint8_t offset2, bool autoEOI) override;
    virtual void Disable() override;
    virtual void SendEOI(int) override;
    virtual void Mask(int) override;
    virtual void Unmask(int) override;
    virtual const char* Name() override { return "8259 PIC"; }

private:

    const uint16_t PIC1CommandPort{ 0x20 };
    const uint16_t PIC1DataPort{ 0x21 };
    const uint16_t PIC2CommandPort{ 0xA0 };
    const uint16_t PIC2DataPort{ 0xA1 };

    uint16_t ReadInServiceRegister();
    uint16_t ReadIRQRequestRegister();
    uint16_t GetMask();
    void SetMask(int);
    uint16_t m_PICMask = 0xFFFF;
    bool m_AutoEOI = false;
};