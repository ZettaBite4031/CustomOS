#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>
#include <algorithm>

#include "IOAllocator.hpp"
#include <core/Debug.hpp>

class PagingManager;

class PCIDevice;
class GeneralPCIDevice;
class PCIPCIBridge;
class PCICardBusBridge;

class PCI {
public:
    enum class Type : uint8_t {
        General     = 0,
        PCIPCI      = 1,
        PCICardBus  = 2,
    };

    static constexpr uint32_t PCI_CONFIG_ADDRESS{ 0xCF8 };
    static constexpr uint32_t PCI_DATA_ADDRESS  { 0xCFC };

    explicit PCI(IORange& range);

    PCIDevice* FindDevice(uint16_t vendor, uint16_t device);

    friend class PCIDevice;

private:
    void     SelectAddress(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const;
    uint32_t ReadConfig   (uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const;
    void     WriteConfig  (uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) const;

    IORange&                 m_Range;
    std::vector<PCIDevice*>  m_Devices;   // owns no memory; we keep base devices separately
};

class PCIDevice {
public:
    struct MmapRange{ uint8_t* start; std::size_t length; };

    static constexpr uint8_t PCI_STATUS_COMMAND_OFFSET{ 0x04 };

    PCIDevice();
    PCIDevice(PCI* parent, uint8_t bus, uint8_t dev, uint16_t ven_id, uint16_t dev_id);

    PCIDevice* Upgrade();

    void PrintIDs() const {
        Debug::Info("PCI", "Ven: 0x%04X | Dev: 0x%04X", m_VendorID, m_DeviceID);
    }

    virtual uint32_t ReadRegister(uint8_t reg);
    virtual void     WriteRegister(uint8_t reg, uint32_t val);
    virtual void     EnableBusMastering();

    PCI*     m_Parent{nullptr};
    uint8_t  m_Bus{0}, m_Device{0}, m_Function{0};
    uint16_t m_VendorID{0}, m_DeviceID{0};
    uint8_t  m_ClassCode{0}, m_Subclass{0}, m_ProgIF{0};
};

class GeneralPCIDevice : public PCIDevice {
public:
    GeneralPCIDevice() = delete;
    explicit GeneralPCIDevice(PCIDevice* base) { *static_cast<PCIDevice*>(this) = *base; }

    uint32_t FindIOBase();
    MmapRange FindMmapRange(PagingManager& KernelPagingManager);
    uint8_t GetIRQ();

    void EnableBusMastering() override { PCIDevice::EnableBusMastering(); }

private:
    uint32_t ReadRegister(uint8_t reg) override { return PCIDevice::ReadRegister(reg); }
    void     WriteRegister(uint8_t reg, uint32_t val) override { PCIDevice::WriteRegister(reg, val); }

    PCIDevice Upgrade() = delete;
};

class PCIPCIBridge : public PCIDevice {
public:
    PCIPCIBridge() = delete;
    explicit PCIPCIBridge(PCIDevice* base) { *static_cast<PCIDevice*>(this) = *base; }

private:
    PCIDevice Upgrade() = delete;
};

class PCICardBusBridge : public PCIDevice {
public:
    PCICardBusBridge() = delete;
    explicit PCICardBusBridge(PCIDevice* base) { *static_cast<PCIDevice*>(this) = *base; }

private:
    PCIDevice Upgrade() = delete;
};