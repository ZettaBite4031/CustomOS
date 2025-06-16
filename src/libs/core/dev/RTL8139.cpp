#include "RTL8139.hpp"

#include <core/arch/i686/Timer.hpp>
#include <core/std/vector.hpp>

RTL8139::RTL8139(GeneralPCIDevice* pci_dev, PCIDevice::MmapRange rtl_mmap, bool loopback) 
    : PCI{ pci_dev }, MMapRange{ rtl_mmap },m_Loopback{ loopback } {
    if (MMapRange.length != 256) {
        Debug::Critical("RTL8139", "Memory map range not 256! Actual: %u", MMapRange.length);
        return;
    }

    pci_dev->EnableBusMastering();

    ResetDevice();
    InitReceiveBuffer();
    InitInterrupts();
    EnableTXRX();

    if (m_Loopback) EnableLoopback();

    InitReceiveConfiguration();
    InitCapr();
}

std::vector<uint8_t> RTL8139::GetMACAddress() {
    std::vector<uint8_t> mac{ 6 };
    for (int i = 0; i < 6; i++) {
        mac[i] = *(MMapRange.start + i);
    }
    return mac;
}

void RTL8139::ResetDevice() {
    volatile uint8_t* command_reg = MMapRange.start + COMMAND_REGISTER_OFFSET;
    uint8_t value = *command_reg;
    value |= (1 << 4);
    *command_reg = value;
    while((*command_reg) & 0x08) {}
}

void RTL8139::InitReceiveBuffer() {
    GenerateRXBuffer();
    WriteRXBufferAddress();
    SetRXBufferSize();
}

void RTL8139::ClearInterrupts() {
    volatile uint16_t* reg = (uint16_t*)(MMapRange.start + INTERRUPT_STATUS_OFFSET);
    uint16_t _ = *reg;
    *reg = 0x05;
}

void RTL8139::SetInterruptMask() {
    volatile uint16_t* int_mask_reg = (uint16_t*)(MMapRange.start + INTERRUPT_MASK_OFFSET);
    *int_mask_reg = 0x05;
}

void RTL8139::InterruptHandler(ISR::Registers* regs, void* data) {
    RTL8139* dev = (RTL8139*)data;
    dev->ClearInterrupts();
}

void RTL8139::InitInterrupts() {
    uint8_t irq = PCI->GetIRQ();

    IRQ::RegisterHandler(irq, InterruptHandler);
    SetInterruptMask();
}

void RTL8139::EnableTXRX() {
    volatile uint8_t* reg = MMapRange.start + COMMAND_REGISTER_OFFSET;
    uint8_t val = *reg;
    val = std::set_bits<uint8_t>(val, 2, 2, 0b11);
    *reg = val;
    uint8_t read_back = *reg;
    if (read_back != val) {
        Debug::Critical("RTL8139", "Value not set for enabling TXRX!");
    }
}

void RTL8139::EnableLoopback() {
    volatile uint32_t* tx_config_reg = (uint32_t*)(MMapRange.start + TRANSMIT_CONFIG_OFFSET);
    uint32_t config = *tx_config_reg;

    config = std::set_bits<uint32_t>(config, 17, 2, 0b11);

    *tx_config_reg = config;

    uint32_t new_val = *tx_config_reg;
    if (new_val != config) {
        Debug::Critical("RTL8139", "Loopback configuration not set!");
    }
}

void RTL8139::InitReceiveConfiguration() {
    volatile uint32_t* global_rx_config_reg = (uint32_t*)(MMapRange.start + RECEIVE_CONFIG_OFFSET);
    uint32_t global_rx_config = *global_rx_config_reg;
    global_rx_config = std::set_bits<uint32_t>(global_rx_config, 0, 6, 0x3F);
    *global_rx_config_reg = global_rx_config;

    uint32_t new_val = *global_rx_config_reg;
    if (new_val != global_rx_config) {
        Debug::Critical("RTL8139", "RX Configuration not set!");
    }
}

void RTL8139::InitCapr() {
    volatile uint16_t* capr = (uint16_t*)(MMapRange.start + CAPR_OFFSET);
    constexpr uint16_t INITIAL_VAL = 0xFFF0;
    *capr = INITIAL_VAL;
    
    uint16_t new_val = *capr;
    if (new_val != INITIAL_VAL) {
        Debug::Critical("RTL8139", "Capr not set!");
    }
}

void RTL8139::GenerateRXBuffer() {
    constexpr size_t DATA_SIZE = 64 * 1024;
    constexpr size_t OVERHEAD = 16;
    constexpr size_t WRAP_PADDING = 1536;
    constexpr size_t BUFFER_SIZE = DATA_SIZE + OVERHEAD + WRAP_PADDING;
    m_RXBuffer = new uint8_t[BUFFER_SIZE];
}

void RTL8139::WriteRXBufferAddress() {
    volatile uint32_t* rbstart = (uint32_t*)(MMapRange.start + RBSTART_OFFSET);
    uint32_t address = reinterpret_cast<uint32_t>(m_RXBuffer);
    *rbstart = address;
    uint32_t new_val = *rbstart;
    if (new_val != address) {
        Debug::Critical("RTL8139", "RX Buffer Address not set!");
    }
}

void RTL8139::SetRXBufferSize() {
    volatile uint32_t* rx_config_reg = (uint32_t*)(MMapRange.start + RECEIVE_CONFIG_OFFSET);
    uint32_t rx_config_val = *rx_config_reg;
    rx_config_val |= (3 << 11);
    *rx_config_reg = rx_config_val;

    uint32_t new_val = *rx_config_reg;
    if (new_val != rx_config_val) {
        Debug::Critical("RTL8139", "RX Buffer Size not set!");
    }
}

void RTL8139::Wait(volatile uint16_t* capr_reg, volatile uint16_t* cbr_reg) {
    while (true) {
        uint16_t capr = *capr_reg;
        capr += 16; // RTL8139 requires CAPR to be 16 bytes behind CBR
        uint16_t cbr = *cbr_reg;

        if (capr != cbr)
            break;

        sleep(1); // yield CPU
    }
}

std::slice<uint8_t> RTL8139::GetPacket() {
    volatile uint16_t* capr_reg = (uint16_t*)(MMapRange.start + CAPR_OFFSET);
    volatile uint16_t* cbr_reg = (uint16_t*)(MMapRange.start + CBR_OFFSET);
    Wait(capr_reg, cbr_reg);

    uint16_t capr = *(uint16_t*)(MMapRange.start + CAPR_OFFSET);
    uint16_t cbr = *(uint16_t*)(MMapRange.start + CBR_OFFSET);
    uint16_t start_offset = capr + 16;
    volatile uint16_t* header = (uint16_t*)(m_RXBuffer + start_offset);
    header++;
    uint16_t length = (*header) - 4;

    uint16_t* start = const_cast<uint16_t*>(header) + 1;
    std::slice<uint8_t> slice = std::slice<uint8_t>((uint8_t*)start, (size_t)length);
    return slice;
}


void RTL8139::IncrementCapr() {
    volatile uint16_t* capr_reg = (uint16_t*)(MMapRange.start + CAPR_OFFSET);
    uint16_t capr = *capr_reg;
    uint16_t start_offset = capr + 16;

    volatile uint16_t* header = (uint16_t*)(m_RXBuffer + start_offset);
    header++;
    uint16_t length = *header;

    capr += (length + 4 + 3) & ~0b11;
    *capr_reg = capr;
}

