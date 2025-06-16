#include "Disk.hpp"

#include <core/cpp/String.hpp>
#include <core/cpp/Algorithm.hpp>


void DecodeATAString(const char* src, char* dst, int length) {
    for (int i = 0; i < length; i += 2) {
        dst[i] = src[i + 1];
        dst[i + 1] = src[i];
    }
    dst[length] = '\0';
}

Disk::Disk(uint32_t drive_id, IORange* range, bool ranged) : m_DriveID{ drive_id }, m_Range{ range }, m_UsedByRangedDevice{ ranged } {}

size_t Disk::Read(uint8_t* data, size_t size) {
    size_t initialPosition = m_Position;
    if (!m_Initialized) {
        ReadNextSector();
        if (!m_UsedByRangedDevice) m_Position = 0;
        m_Initialized = true;
    }
    if (m_Position >= m_Size) return 0;

    while (size > 0) {
        size_t bufferPos = m_Position % BytesPerSector;
        size_t canRead = min(size, BytesPerSector - bufferPos);
        size_t remaining = m_Size - m_Position;
        canRead = min(canRead, remaining);
        Memory::Copy(data, m_Buffer + bufferPos, canRead);
        size -= canRead;
        data += canRead;
        m_Position += canRead;

        if (size > 0 && !ReadNextSector()) break;
    }

    return m_Position - initialPosition;
}

size_t Disk::Write(const uint8_t* data, size_t size) {
    size_t initialPosition = m_Position;
    if (m_Position >= m_Size) return 0;

    while (size > 0) {
        size_t bufferPos = m_Position % BytesPerSector;
        size_t canWrite = min(size, BytesPerSector - bufferPos);
        size_t remaining = m_Size - m_Position;
        canWrite = min(canWrite, remaining);

        // load current sector if needed
        if (!ReadNextSector()) break;

        Memory::Copy(m_Buffer + bufferPos, data, canWrite);

        if (!WriteCurrentSector()) break;

        size -= canWrite;
        data += canWrite;
        m_Position += canWrite;
    }

    return m_Position - initialPosition;
}

bool Disk::Seek(int rel, SeekPos pos) {
    uint32_t newPos = 0;
    switch (pos) {
        case SeekPos::Set:      newPos = rel; break;
        case SeekPos::Current:  newPos = m_Position + rel; break;
        case SeekPos::End:      newPos = m_Size - rel; break;
    }

    if (newPos > m_Size || newPos < 0) return false;
    m_Position = newPos;
    return ReadNextSector();
}

size_t Disk::Position() {
    return m_Position;
}

size_t Disk::Size() {
    return m_Size;
}

bool CheckATAIdentify(ATAIdentifyDevice& id) {
    if (id.GeneralConfig & 0x8000) return false;
    if (id.LBA28SectorCount == 0 && id.LBA48SectorCount == 0) return false;
    char model[41];
    DecodeATAString((char*)id.ModelNumber, model, 40);
    for (int i = 0; i < 40; i++) {
        if (model[i] < 0x20 || model[i] > 0x7E)
        return false;
    }
    return true;
}

bool Disk::Initialize() {
    m_Range->write<uint8_t>(0x06, 0xA0);
    m_Range->write<uint8_t>(0x01, 0x00);
    m_Range->write<uint8_t>(0x02, 0x00);
    m_Range->write<uint8_t>(0x03, 0x00);
    m_Range->write<uint8_t>(0x04, 0x00);
    m_Range->write<uint8_t>(0x05, 0x00);
    m_Range->write<uint8_t>(0x07, 0xEC);
    
    uint8_t status;
    do {
        status = m_Range->read<uint8_t>(0x7);
    } while (status & 0x80);
    if (!(status & 0x08)) {
        Debug::Critical("ATA", "IDENTIFY failed or is not supported!");
        return false;
    }

    uint16_t ata_identify_buf[256];
    for (int i = 0; i < 256; i++) 
        ata_identify_buf[i] = m_Range->read<uint16_t>(0x00);

    memcpy(&m_Configuration, ata_identify_buf, sizeof(ATAIdentifyDevice));

    m_Size = BytesPerSector * m_Configuration.LBA28SectorCount;
    m_Position = -1;

    return CheckATAIdentify(m_Configuration);
}

bool Disk::WaitDRQ() {
    for (int i{ 0 }; i < 1000; i++) {
        uint8_t status = m_Range->read<uint8_t>(0x7);
        if (!(status & 0x80) && (status & 0x08)) return true;
        if (status & 0x01) return false;
        // TODO: Implement a better sleep function that's not specific to the Kernel
    }
    return false;
}

bool Disk::ReadNextSector() {
    uint32_t desiredLBA = m_Position / BytesPerSector;
    if (desiredLBA == m_CurrentLBA) return true;
    m_CurrentLBA = desiredLBA;

    while (m_Range->read<uint8_t>(0x7) & 0x80) 
        for (int i{ 0 }; i < 5000; i++)
            ; // TODO: Better sleep function

    m_Range->write<uint8_t>(0x6, 0xE0 | (m_DriveID << 4) | ((m_CurrentLBA >> 24) & 0x0F));

    m_Range->write<uint8_t>(0x2, (uint8_t)1); // read one sector
    m_Range->write<uint8_t>(0x3, (uint8_t)(m_CurrentLBA & 0xFF));
    m_Range->write<uint8_t>(0x4, (uint8_t)((m_CurrentLBA >> 8) & 0xFF));
    m_Range->write<uint8_t>(0x5, (uint8_t)((m_CurrentLBA >> 16) & 0xFF));

    m_Range->write<uint8_t>(0x7, 0x20);

    uint16_t* u16buf = (uint16_t*)m_Buffer;
    if (!WaitDRQ()) {
        Debug::Critical("Disk", "WaitDRQ failed!");
        return false;
    }
    for (int i{ 0 }; i < 256; i++)
        u16buf[i] = m_Range->read<uint16_t>(0x0);

    return true;
}

bool Disk::WriteCurrentSector() {
    while (m_Range->read<uint8_t>(0x7) & 0x80) 
        for (int i{ 0 }; i < 5000; i++)
            ; // TODO: Better sleep function

    m_Range->write<uint8_t>(0x06, 0xE0 | (m_DriveID << 4) | ((m_CurrentLBA >> 24) & 0x0F));
    m_Range->write<uint8_t>(0x02, 1); // read a single sector
    m_Range->write<uint8_t>(0x03, (uint8_t)(m_CurrentLBA & 0xFF));
    m_Range->write<uint8_t>(0x04, (uint8_t)((m_CurrentLBA >> 8) & 0xFF));
    m_Range->write<uint8_t>(0x05, (uint8_t)((m_CurrentLBA >> 16) & 0xFF));
    m_Range->write<uint8_t>(0x07, 0x30); // WRITE SECTORS

    if (!WaitDRQ()) {
        Debug::Critical("Disk", "WaitDRQ failed!");
        return false;
    }

    uint16_t* buf = reinterpret_cast<uint16_t*>(m_Buffer);
    for (int i = 0; i < 256; i++) {
        m_Range->write<uint16_t>(0x0, buf[i]);
    }

    return true;
}