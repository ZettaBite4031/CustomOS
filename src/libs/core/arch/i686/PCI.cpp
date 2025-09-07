#include "PCI.hpp"

PCI::PCI(IORange* range) 
    : m_Range{ range } {

}

PCIDevice PCI::FindDevice(uint16_t vendor, uint16_t device) {
    size_t idx = m_Devices.find([vendor, device](PCIDevice* dev){
        return dev->m_VendorID == vendor && dev->m_DeviceID == device;
    });
    if (idx != m_Devices.size()) {
        Debug::Critical("PCI", "The PCIDevice 0x%X|0x%X is already in use", vendor, device);
        return PCIDevice();
    }
    
    for (uint8_t dev{ 0 }; dev < 8; dev++) {
        for (uint8_t bus{ 0 }; bus < 32; bus++) {
            for (uint8_t func{ 0 }; func < 8; func++) {
                uint16_t vid = ReadConfig(bus, dev, func, 0) & 0xFFFF;
                if (vid == 0xFFFF) continue; // no device here
                uint16_t did = (ReadConfig(bus, dev, func, 2) >> 16) & 0xFFFF;

                if (vendor == vid && device == did) {
                    return PCIDevice(this, bus, dev, vid, did);
                }
            }
        }
    }

    Debug::Error("PCI", "Could not find PCIDevice 0x%X|0x%X", vendor, device);
    return PCIDevice();
}

void PCI::SelectAddress(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const {
    uint32_t address = 0x80000000 | (bus << 16) | (dev << 11) | (func << 8) | (offset & 0xFC);
    m_Range->write<uint32_t>(0, address);
}

uint32_t PCI::ReadConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const {
    SelectAddress(bus, dev, func, offset);
    return m_Range->read<uint32_t>(4);
}

void PCI::WriteConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) const {
    SelectAddress(bus, dev, func, offset);
    m_Range->write<uint32_t>(4, value);
}

// ---------------------------------------------------------------------------

PCIDevice::PCIDevice() 
    : m_Parent{ nullptr }, m_Bus{ 0 }, m_Device{ 0 }, m_Function{ 0 },
      m_VendorID{ 0 }, m_DeviceID{ 0 }, m_ClassCode{ 0 }, m_Subclass{ 0 },
      m_ProgIF{ 0 } {};

PCIDevice::PCIDevice(PCI* parent, uint8_t bus, uint8_t dev, uint16_t ven_id, uint16_t dev_id)
    : m_Parent{ parent }, m_Bus{ bus }, m_Device{ dev }, m_Function{ 0 },
      m_VendorID{ ven_id }, m_DeviceID{ dev_id }, m_ClassCode{ 0 }, m_Subclass{ 0 },
      m_ProgIF{ 0 } {};

uint32_t PCIDevice::ReadRegister(uint8_t reg) {
    return m_Parent->ReadConfig(m_Bus, m_Device, 0, reg * 4);
}

void PCIDevice::WriteRegister(uint8_t reg, uint32_t val) {
    m_Parent->WriteConfig(m_Bus, m_Device, 0, reg * 4, val);
}

void PCIDevice::EnableBusMastering() {
    uint32_t status_command = m_Parent->ReadConfig(m_Bus, m_Device, 0, PCI_STATUS_COMMAND_OFFSET);
    status_command |= 1 << 2;
    m_Parent->WriteConfig(m_Bus, m_Device, 0, PCI_STATUS_COMMAND_OFFSET, status_command);
}

PCIDevice* PCIDevice::Upgrade() {
    uint8_t header_type = (uint8_t)(ReadRegister(3) >> 8);

    switch (header_type) {
        case 0: return new GeneralPCIDevice(this);
        case 1: return new PCIPCIBridge(this);
        case 2: return new PCICardBusBridge(this);
        default: return {};
    }
}

GeneralPCIDevice::GeneralPCIDevice(PCIDevice* base) {
    static_assert(std::is_same<decltype(*base), PCIDevice&>::value, 
        "Specific PCI Devices cannot be instantiated with other specific PCI Devices!");
    m_Base = base;
}

uint32_t GeneralPCIDevice::FindIOBase() {
    for (int i = 0; i < 5; i++) {
        uint32_t base_address = ReadRegister(4 + i);
        if ((base_address & 0x1) && (base_address >> 1) != 0) {
            return base_address & ~0b11;
        }
    }

    return 0;
}

PCIDevice::MmapRange GeneralPCIDevice::FindMmapRange(PagingManager& KernelPagingManger) {
    for (int i = 0; i < 5; i++) {
        uint8_t reg_off = 4 + i;
        uint32_t base_address = ReadRegister(reg_off);
        if ((base_address & 0x1) == 0 && base_address > 0) {
            uintptr_t phys_start = base_address & ~0xFULL;
            
            WriteRegister(reg_off, ~0);
            uint32_t end_address = ReadRegister(reg_off);

            size_t length = ~(end_address & ~0xFULL) + 1;
            WriteRegister(reg_off, base_address);

            uintptr_t virt_start = 0xF0000000 + (i * 0x100000);
            KernelPagingManger.MapRange(phys_start, virt_start, length, PAGE_PRESENT | PAGE_READWRITE);

            return { reinterpret_cast<uint8_t*>(virt_start), length };
        }
    }

    return { nullptr, 0 };
}

uint8_t GeneralPCIDevice::GetIRQ() {
    uint32_t reg = ReadRegister(0xF);
    uint8_t irq = reg & 0xFF;
    return irq;
}


PCIPCIBridge::PCIPCIBridge(PCIDevice* base) {
    static_assert(std::is_same<decltype(*base), PCIDevice&>::value, 
        "Specific PCI Devices cannot be instantiated with other specific PCI Devices!");
    m_Base = base;
}

PCICardBusBridge::PCICardBusBridge(PCIDevice* base) {
    static_assert(std::is_same<decltype(*base), PCIDevice&>::value, 
        "Specific PCI Devices cannot be instantiated with other specific PCI Devices!");
    m_Base = base;
}