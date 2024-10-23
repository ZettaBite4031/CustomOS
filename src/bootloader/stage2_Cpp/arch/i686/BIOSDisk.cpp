#include "BIOSDisk.hpp"
#include "RealMemory.hpp"

#include <core/cpp/Algorithm.hpp>
#include <core/cpp/Memory.hpp>
#include <core/ZosDefs.hpp>
#include <core/Assert.hpp>
#include <core/Debug.hpp>

namespace {
    struct ExtendedDriveParameters {
        uint16_t ParamsSize;
        uint16_t Flags;
        uint32_t Cylinders;
        uint32_t Heads;
        uint32_t SectorsPerTrack;
        uint64_t Sectors;
        uint16_t BytesPerSector;
    } PACKED;

    struct ExtendedReadParameters {
        uint8_t ParamsSize;
        uint8_t _Reserved;
        uint16_t Count;
        uint32_t Buffer;
        uint64_t LBA;
    } PACKED;

    EXPORT bool ASMCALL i686_DiskReset(uint8_t drive);

    EXPORT bool ASMCALL i686_DiskRead(uint8_t drive,
                                      uint16_t cylinder,
                                      uint16_t sector,
                                      uint16_t head,
                                      uint8_t count,
                                      uint8_t* lowerDataOut);

    EXPORT bool ASMCALL i686_GetDiskDriveParams(uint8_t drive,
                                                uint8_t* driveType,
                                                uint16_t* cylinders, 
                                                uint16_t* sectors,
                                                uint16_t* heads);

    EXPORT bool ASMCALL i686_CheckDiskExtensionsPresent(uint8_t drive);
    EXPORT bool ASMCALL i686_ExtendedDiskRead(uint8_t drive, ExtendedReadParameters* erp);
    EXPORT bool ASMCALL i686_GetExtendedDiskDriveParams(uint8_t drive, ExtendedDriveParameters* edr);
} // anonymous namespace 

BIOSDisk::BIOSDisk(uint8_t id) 
    : m_ID(id), m_Position(-1), m_Size(0) { }
    
bool BIOSDisk::Initialize() {
    m_HasExtensions = i686_CheckDiskExtensionsPresent(m_ID);
    if (m_HasExtensions) {
        ExtendedDriveParameters params;
        params.ParamsSize = sizeof(ExtendedDriveParameters);
        if (!i686_GetExtendedDiskDriveParams(m_ID, &params)) 
            return false;
        Assert(params.BytesPerSector == BIOSSectorSize);
        m_Size = BIOSSectorSize * params.Sectors;
    }
    else {
        if (!i686_GetDiskDriveParams(m_ID, &m_DriveType, &m_Cylinders, &m_Sectors, &m_Heads))
            return false;
    }
    return true;
}
size_t BIOSDisk::Read(uint8_t* data, size_t size) {
    size_t initialPosition = m_Position;
    if (m_Position == -1) {
        initialPosition++;
        ReadNextSector();
        m_Position = 0;
    }
    if (m_Position >= m_Size) return 0;

    while (size > 0) {
        size_t bufferPos = m_Position % BIOSSectorSize;
        size_t canRead = min(size, BIOSSectorSize - bufferPos);
        Memory::Copy(data, m_Buffer + bufferPos, canRead);
        size -= canRead;
        data += canRead;
        m_Position += canRead;

        if (size > 0) ReadNextSector();
    }
    Debug::Debug("BIOSDisk - Read", "Position: %lu, Initial Position: %lu, Size: %lu", m_Position, initialPosition, m_Size);
    return m_Position - initialPosition;
}

bool BIOSDisk::ReadNextSector() {
    uint64_t lba = m_Position / BIOSSectorSize;
    bool ok = false;

    if (m_HasExtensions) {
        ExtendedReadParameters params;
        params.ParamsSize = sizeof(ExtendedDriveParameters);
        params._Reserved = 0x0;
        params.Count = 1;
        params.Buffer = ToSegOffset(m_Buffer);
        params.LBA = lba;

        for (int i = 0; i < 3 && !ok; i++) {
            ok = i686_ExtendedDiskRead(m_ID, &params);
            if (!ok) i686_DiskReset(m_ID);
        }
    } else {
        uint16_t cyl, sec, head;
        LBA2CHS(lba, &cyl, &sec, &head);

        for (int i = 0; i < 3 && !ok; i++) {
            ok = i686_DiskRead(m_ID, cyl, sec, head, 1, m_Buffer);
            if (!ok) i686_DiskReset(m_ID);
        }
    }

    return ok;
}

size_t BIOSDisk::Write(const uint8_t* data, size_t size) {
    return -1;
}

bool BIOSDisk::Seek(int rel, SeekPos pos) {
    bool ok = true;
    switch (pos) {
        case SeekPos::Set:
            m_Position = rel;
            break;
        case SeekPos::Current:
            m_Position += rel;
            ok = ReadNextSector();
            break;
        case SeekPos::End:
            m_Position = m_Size - rel;
            break;

    }
    Debug::Debug("BIOSDisk - Seek", "Position: %lu", m_Position);
    return ok;
}

size_t BIOSDisk::Position() {
    return m_Position;
}

size_t BIOSDisk::Size() {
    return m_Size;
}

void BIOSDisk::LBA2CHS(uint32_t LBA, uint16_t* cyl, uint16_t* sec, uint16_t* head) {
    *sec = LBA % m_Sectors + 1;
    *cyl = (LBA / m_Sectors) / m_Heads;
    *head = (LBA / m_Sectors) % m_Heads;
}
