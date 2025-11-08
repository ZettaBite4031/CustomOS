#include "PCI.hpp"
#include <core/arch/i686/PagingManager.hpp>

PCI::PCI(IORange& range) : m_Range{ range } {}

PCIDevice* PCI::FindDevice(uint16_t vendor, uint16_t device) {
    auto it = std::find_if(m_Devices.begin(), m_Devices.end(),
            [&](const PCIDevice* dev) {
                return dev->m_VendorID == vendor && dev->m_DeviceID == device;
            });

    if (it != m_Devices.end()) {
        Debug::Critical("PCI", "PCIDevice 0x%04X|0x%04X is already in use", vendor, device);
        return nullptr;
    }

    for (uint8_t bus = 0; bus < 256; ++bus) {
        for (uint8_t dev = 0; dev < 32; ++dev) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint32_t id = ReadConfig(bus, dev, func, 0x00);
                uint16_t vid = static_cast<uint16_t>(id & 0xFFFF);
                if (vid == 0xFFFF) continue; // no device present

                uint16_t did = static_cast<uint16_t>((id >> 16) & 0xFFFF);
                if (vendor == vid && device == did) {
                    // create a base device and track it; keep function 0 by default
                    auto* base = new PCIDevice(this, bus, dev, vid, did);
                    m_Devices.push_back(base);
                    return base;
                }
            }
        }
    }    

    Debug::Error("PCI", "Could not find PCIDevice  0x%04X|0x%04X", vendor, device);
    return nullptr;
}

void PCI::SelectAddress(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const {
    const uint32_t address =
        0x80000000u
      | (static_cast<uint32_t>(bus)  << 16)
      | (static_cast<uint32_t>(dev)  << 11)
      | (static_cast<uint32_t>(func) << 8)
      | (static_cast<uint32_t>(offset) & 0xFCu);

    m_Range.write<uint32_t>(0, address); // 0xCF8
}

uint32_t PCI::ReadConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset) const {
    SelectAddress(bus, dev, func, offset);
    return m_Range.read<uint32_t>(4);     // 0xCFC
}

void PCI::WriteConfig(uint8_t bus, uint8_t dev, uint8_t func, uint8_t offset, uint32_t value) const {
    SelectAddress(bus, dev, func, offset);
    m_Range.write<uint32_t>(4, value);    // 0xCFC
}

// ---------------- PCIDevice ----------------

PCIDevice::PCIDevice() = default;

PCIDevice::PCIDevice(PCI* parent, uint8_t bus, uint8_t dev, uint16_t ven_id, uint16_t dev_id)
    : m_Parent{parent}, m_Bus{bus}, m_Device{dev}, m_Function{0},
      m_VendorID{ven_id}, m_DeviceID{dev_id},
      m_ClassCode{0}, m_Subclass{0}, m_ProgIF{0} {}

uint32_t PCIDevice::ReadRegister(uint8_t reg) {
    return m_Parent->ReadConfig(m_Bus, m_Device, 0, static_cast<uint8_t>(reg * 4));
}

void PCIDevice::WriteRegister(uint8_t reg, uint32_t val) {
    m_Parent->WriteConfig(m_Bus, m_Device, 0, static_cast<uint8_t>(reg * 4), val);
}

void PCIDevice::EnableBusMastering() {
    uint32_t status_command = m_Parent->ReadConfig(m_Bus, m_Device, 0, PCI_STATUS_COMMAND_OFFSET);
    status_command |= (1u << 2); // Bus Master Enable
    m_Parent->WriteConfig(m_Bus, m_Device, 0, PCI_STATUS_COMMAND_OFFSET, status_command);
}

PCIDevice* PCIDevice::Upgrade() {
    // Header Type is at offset 0x0E: read dword 3 (offset 0x0C),
    // header type is the byte at 0x0E => (value >> 16) & 0xFF
    const uint8_t header_type = static_cast<uint8_t>((ReadRegister(3) >> 16) & 0xFF);

    switch (header_type & 0x7F) { // bit 7 = multifunction flag
        case 0x00: return new GeneralPCIDevice(this);
        case 0x01: return new PCIPCIBridge(this);
        case 0x02: return new PCICardBusBridge(this);
        default:
            Debug::Error("PCI", "Unknown PCI header type 0x%02X", header_type);
            return nullptr;
    }
}

// ---------------- GeneralPCIDevice ----------------

uint32_t GeneralPCIDevice::FindIOBase() {
    // BAR0..BAR5 at registers 4..9
    for (int i = 0; i < 6; ++i) {
        const uint32_t bar = ReadRegister(static_cast<uint8_t>(4 + i));
        const bool is_io = (bar & 0x1u) != 0;
        if (is_io && ((bar >> 1) != 0)) {
            return bar & ~0x3u; // mask off IO indicator bits
        }
    }
    return 0;
}

PCIDevice::MmapRange GeneralPCIDevice::FindMmapRange(PagingManager& KernelPagingManager) {
    // Memory BARs only (bit0 == 0)
    for (int i = 0; i < 6; ++i) {
        const uint8_t reg_off = static_cast<uint8_t>(4 + i);
        const uint32_t bar = ReadRegister(reg_off);

        const bool is_mem = (bar & 0x1u) == 0;
        if (is_mem && bar != 0) {
            const uintptr_t phys_start = static_cast<uintptr_t>(bar & ~0xFu);

            // Size discovery
            WriteRegister(reg_off, 0xFFFF'FFFFu);
            const uint32_t size_mask = ReadRegister(reg_off);
            WriteRegister(reg_off, bar);

            const std::size_t length = static_cast<std::size_t>(~(size_mask & ~0xFu) + 1u);

            const uintptr_t virt_start = 0xF0000000u + static_cast<uintptr_t>(i) * 0x100000u;
            KernelPagingManager.MapRange(phys_start, virt_start, length, PAGE_PRESENT | PAGE_READWRITE);

            return { reinterpret_cast<uint8_t*>(virt_start), length };
        }
    }
    return { nullptr, 0 };
}

uint8_t GeneralPCIDevice::GetIRQ() {
    const uint32_t reg = ReadRegister(0x0F); // Interrupt Line/Pin
    return static_cast<uint8_t>(reg & 0xFFu);
}