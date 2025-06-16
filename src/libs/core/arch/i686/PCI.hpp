#pragma once

#include <stdint.h>
#include <stddef.h>

#include "IOAllocator.hpp"
#include <core/std/vector.hpp>

class PCIDevice;

class PCI {
public:
    enum Type : uint8_t {
        General = 0,
        PCIPCI = 1,
        PCICardBus = 2,
    };

    static constexpr uint32_t PCI_CONFIG_ADDRESS{ 0xCF8 };
    static constexpr uint32_t PCI_DATA_ADDRESS{ 0xCFC };

    PCI(IORange* range);

    PCIDevice FindDevice(uint16_t vendor, uint16_t device);

    friend class PCIDevice; 
private:
    void SelectAddress(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const; 
    uint32_t ReadConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const;
    void WriteConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) const;


    IORange* m_Range{ nullptr };
    std::vector<PCIDevice*> m_Devices;
};


class PCIDevice {
public:
    struct MmapRange {
        uint8_t* start;
        size_t length;
    };

    static constexpr uint8_t PCI_STATUS_COMMAND_OFFSET{ 0x4 };

    PCIDevice();
    PCIDevice(PCI* parent, uint8_t bus, uint8_t dev, uint16_t ven_id, uint16_t dev_id);

    friend class PCI;

    PCIDevice* Upgrade();
    void PrintIDs() const { Debug::Info("PCI", "Ven: 0x%X | Dev: 0x%X", m_VendorID, m_DeviceID); }
//protected:
    virtual uint32_t ReadRegister(uint8_t reg);
    virtual void WriteRegister(uint8_t reg, uint32_t val);
    virtual void EnableBusMastering();
    
    
    PCI* m_Parent;
    uint8_t m_Bus, m_Device, m_Function;
    uint16_t m_VendorID, m_DeviceID;
    uint8_t m_ClassCode, m_Subclass, m_ProgIF;

};

class GeneralPCIDevice : public PCIDevice {
public:
    GeneralPCIDevice() = delete;
    GeneralPCIDevice(PCIDevice* base);

    uint32_t FindIOBase();
    MmapRange FindMmapRange();
    uint8_t GetIRQ();

    void EnableBusMastering() override { m_Base->EnableBusMastering(); };
private:
    uint32_t ReadRegister(uint8_t reg) override { return m_Base->ReadRegister(reg); }
    void WriteRegister(uint8_t reg, uint32_t val) override { m_Base->WriteRegister(reg, val); }

    PCIDevice Upgrade() = delete;
    PCIDevice* m_Base;
};

class PCIPCIBridge : public PCIDevice {
public:
    PCIPCIBridge() = delete;
    PCIPCIBridge(PCIDevice* base);

private:
    PCIDevice Upgrade() = delete;
    PCIDevice* m_Base;
};

class PCICardBusBridge : public PCIDevice {
public:
    PCICardBusBridge() = delete;
    PCICardBusBridge(PCIDevice* base);

private:
    PCIDevice Upgrade() = delete;
    PCIDevice* m_Base;
};
