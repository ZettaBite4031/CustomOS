#include "RTL8139.hpp"

constexpr size_t COMMAND_REGISTER_OFFSET = 0x37;
constexpr size_t RBSTART_OFFSET = 0x30;
constexpr size_t RECEIVE_CONFIG_OFFSET = 0x44;
constexpr size_t INTERRUPT_MASK_OFFSET = 0x3c;
constexpr size_t INTERRUPT_STATUS_OFFSET = 0x3e;
constexpr size_t TRANSMIT_CONFIG_OFFSET = 0x40;
constexpr size_t TRANSMIT_STATUS_OFFSET = 0x10;
constexpr size_t TRANSMIT_DATA_OFFSET = 0x20;
constexpr size_t CAPR_OFFSET = 0x38;
constexpr size_t CBR_OFFSET = 0x3a;

template<typename T>
T set_bits(T original, uint8_t start, uint8_t length, T value) {
    static_assert(std::is_unsigned<T>::value, "T must be unsigned (for bit safety)");

    if (length == 0 || start >= sizeof(T) * 8 || length > sizeof(T) * 8)
        return original; // No-op or invalid

    // Clamp to valid bit range
    if (start + length > sizeof(T) * 8)
        length = sizeof(T) * 8 - start;

    T mask = ((T{1} << length) - 1) << start;
    T shiftedValue = (value << start) & mask;

    return (original & ~mask) | shiftedValue;
}

RTL8139::RTL8139(GeneralPCIDevice* pci_dev, PCIDevice::MmapRange rtl_mmap, bool loopback) 
    : PCI{ pci_dev }, MMapRange{ rtl_mmap },m_Loopback{ loopback } {
    if (MMapRange.length != 256) {
        Debug::Critical("RTL8139", "Memory map range not 256! Actual: %u", MMapRange.length);
        return;
    }

    pci_dev->EnableBusMastering();

    ResetDevice();
    //auto receive_buf = InitReceiveBuffer();
    InitReceiveBuffer();
    //TODO: !!!! IMPLEMENT KERNEL WIDE GENERIC INTERRUPT HANDLER -> InitInterrupts();
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

void RTL8139::InitInterrupts() {

}

void RTL8139::EnableTXRX() {
    volatile uint8_t* reg = MMapRange.start + COMMAND_REGISTER_OFFSET;
    uint8_t val = *reg;
    val = set_bits<uint8_t>(val, 2, 2, 0b11);
    *reg = val;
    uint8_t read_back = *reg;
    if (read_back != val) {
        Debug::Critical("RTL8139", "Value not set for enabling TXRX!");
    }
}

void RTL8139::EnableLoopback() {
    volatile uint32_t* tx_config_reg = (uint32_t*)(MMapRange.start + TRANSMIT_CONFIG_OFFSET);
    uint32_t config = *tx_config_reg;

    config = set_bits<uint32_t>(config, 17, 2, 0b11);

    *tx_config_reg = config;

    uint32_t new_val = *tx_config_reg;
    if (new_val != config) {
        Debug::Critical("RTL8139", "Loopback configuration not set!");
    }
}

void RTL8139::InitReceiveConfiguration() {
    volatile uint32_t* global_rx_config_reg = (uint32_t*)(MMapRange.start + RECEIVE_CONFIG_OFFSET);
    uint32_t global_rx_config = *global_rx_config_reg;
    global_rx_config = set_bits<uint32_t>(global_rx_config, 0, 6, 0x3F);
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
    m_RXBuffer.reserve(BUFFER_SIZE);
}

void RTL8139::WriteRXBufferAddress() {
    volatile uint32_t* rbstart = (uint32_t*)(MMapRange.start + RBSTART_OFFSET);
    uint32_t address = reinterpret_cast<uint32_t>(m_RXBuffer.data());
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
