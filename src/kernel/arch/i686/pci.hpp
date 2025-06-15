#pragma once

#include <stdint.h>

typedef struct pci_dev {
    uint8_t bus, slot, func;
    uint16_t vendor_id, device_id;
    uint8_t cls, subclass, prog_if;
    uint32_t bar[6];
} pci_dev_t;


static inline uint32_t PCI_ConfigRead(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
static inline void PCI_ConfigWrite(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);
void PCI_Enumerate();
void PCI_GetRTL8139(pci_dev_t& dev);