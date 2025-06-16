#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/arch/i686/PCI.hpp>
#include <core/std/vector.hpp>

class RTL8139 {
public:
    RTL8139(GeneralPCIDevice* pci_dev, PCIDevice::MmapRange rtl_mmap, bool loopback = false);

    std::vector<uint8_t> GetMACAddress();
private:
    void ResetDevice();
    void InitReceiveBuffer();
    void InitInterrupts();
    void EnableTXRX();
    void EnableLoopback();
    void InitReceiveConfiguration();
    void InitCapr();

    void GenerateRXBuffer();
    void WriteRXBufferAddress();
    void SetRXBufferSize();

    GeneralPCIDevice* PCI{ nullptr };
    PCIDevice::MmapRange MMapRange{ nullptr, 0 };
    bool m_Loopback{ false };
    std::vector<uint8_t> m_RXBuffer;
};