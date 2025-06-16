#pragma once

#include <stdint.h>
#include <stddef.h>

#include <core/arch/i686/PCI.hpp>
#include <core/std/vector.hpp>

#include <core/arch/i686/IRQ.hpp>
#include <core/std/Utility.hpp>

#include <core/arch/i686/Timer.hpp>

class RTL8139 {
public:
    RTL8139(GeneralPCIDevice* pci_dev, PCIDevice::MmapRange rtl_mmap, bool loopback = false);

    template<typename Func>
    void read(Func on_read) {
        auto data = GetPacket();
        on_read(data);
        IncrementCapr();
    }

    template<typename T>
    void write(std::slice<T>& packet) {
        if (packet.size() < 64) {
            Debug::Error("RTL8139", "Packet is too small!");
            return;
        }

        size_t extra_offset = m_TransmitIndex * sizeof(uint32_t);
        size_t data_offset = TRANSMIT_DATA_OFFSET + extra_offset;
        size_t status_offset = TRANSMIT_STATUS_OFFSET + extra_offset;
        volatile uint32_t* data_ptr = (uint32_t*)(MMapRange.start + data_offset);
        volatile uint32_t* status_ptr = (uint32_t*)(MMapRange.start + status_offset);
        TransmitAndWait(data_ptr, status_ptr, packet);
        m_TransmitIndex = (m_TransmitIndex + 1) % 4; 
    }

    std::vector<uint8_t> GetMACAddress();
private:
    static constexpr size_t COMMAND_REGISTER_OFFSET = 0x37;
    static constexpr size_t RBSTART_OFFSET = 0x30;
    static constexpr size_t RECEIVE_CONFIG_OFFSET = 0x44;
    static constexpr size_t INTERRUPT_MASK_OFFSET = 0x3c;
    static constexpr size_t INTERRUPT_STATUS_OFFSET = 0x3e;
    static constexpr size_t TRANSMIT_CONFIG_OFFSET = 0x40;
    static constexpr size_t TRANSMIT_STATUS_OFFSET = 0x10;
    static constexpr size_t TRANSMIT_DATA_OFFSET = 0x20;
    static constexpr size_t CAPR_OFFSET = 0x38;
    static constexpr size_t CBR_OFFSET = 0x3a;

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

    void ClearInterrupts();
    void SetInterruptMask();

    std::slice<uint8_t> GetPacket();
    void IncrementCapr();
    void Wait(volatile uint16_t* capr_reg, volatile uint16_t* cbr_reg);
    
    static void InterruptHandler(ISR::Registers* regs, void* data);

    template<typename T>
    void TransmitAndWait(volatile uint32_t* data_ptr, volatile uint32_t* status_ptr, std::slice<T>& packet) {
        *data_ptr = (uint32_t)packet.data();

        uint32_t status = *status_ptr;
        status = std::set_bits<uint32_t>(status, 0, 12, packet.size());
        status = std::set_bits<uint32_t>(status, 13, 1, 0);
        *status_ptr = status;

        while (true) {
            status = *status_ptr;
            uint32_t own = (status >> 13) & 0x1;

            if (own) break;

            sleep(1);
        }
    }

    GeneralPCIDevice* PCI{ nullptr };
    PCIDevice::MmapRange MMapRange{ nullptr, 0 };
    bool m_Loopback{ false };
    uint8_t* m_RXBuffer;
    uint32_t m_TransmitIndex{ 0 };
};