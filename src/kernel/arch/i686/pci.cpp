#include "pci.hpp"
#include "io.h"


#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define PCI_VENDOR_ID_OFFSET    0x00
#define PCI_DEVICE_ID_OFFSET    0x02
#define PCI_CMD_OFFSET          0x04
#define PCI_CLASS_CODE_OFFSET   0x0B
#define PCI_BAR0_OFFSET         0x10

static inline uint32_t PCI_ConfigRead(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = (1u << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    i686_OutL(PCI_CONFIG_ADDR, addr);
    return i686_InL(PCI_CONFIG_DATA);
}

static inline void PCI_ConfigWrite(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t addr = (1u << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC);
    i686_OutL(PCI_CONFIG_ADDR, addr);
    i686_OutL(PCI_CONFIG_DATA, value);
}

pci_dev_t pci_dev_RTL8139 = {0};
void PCI_Enumerate() {
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            for (uint8_t func = 0; func < 8; func++) {
                uint16_t vid = PCI_ConfigRead(bus, slot, func, PCI_VENDOR_ID_OFFSET) & 0xFFFF;
                if (vid == 0xFFFF) continue; // no device

                pci_dev_t dev = { bus, slot, func };
                dev.vendor_id = vid;
                dev.device_id = (PCI_ConfigRead(bus, slot, func, PCI_DEVICE_ID_OFFSET) >> 16) & 0xFFFF;
                uint32_t cls = PCI_ConfigRead(bus, slot, func, PCI_CLASS_CODE_OFFSET);
                dev.cls       = (cls >> 24) & 0xFF;
                dev.subclass    = (cls >> 16) & 0xFF;
                dev.prog_if     = (cls >> 8) & 0xFF;

                for (int i = 0; i < 6; i++) {
                    dev.bar[i] = PCI_ConfigRead(bus, slot, func, PCI_BAR0_OFFSET + i*4);
                }

                if (dev.vendor_id == 0x10EC && dev.device_id == 0x8139) pci_dev_RTL8139 = dev;
            }
        }
    }
}

void PCI_GetRTL8139(pci_dev_t& dev) {
    dev = pci_dev_RTL8139;
}