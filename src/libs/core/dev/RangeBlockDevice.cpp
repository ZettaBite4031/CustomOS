#include "RangeBlockDevice.hpp"

#include <core/Debug.hpp>
#include <core/cpp/Algorithm.hpp>

RangeBlockDevice::RangeBlockDevice()
    : m_Device(nullptr), m_RangeBegin(0), m_RangeSize(0) {}

void RangeBlockDevice::Initialize(BlockDevice* device, size_t rangeBegin, size_t rangeSize) {
    m_Device = device;
    m_RangeBegin = rangeBegin;
    m_RangeSize = rangeSize;
    m_Device->Seek(rangeBegin, SeekPos::Set);
}

size_t RangeBlockDevice::Read(uint8_t* data, size_t size) {
    if (!m_Device) return -1;
    size = min(size, Size() - Position());
    return m_Device->Read(data, size);
}

size_t RangeBlockDevice::Write(const uint8_t* data, size_t size) {
    if (!m_Device) return -1;
    size = min(size, Size() - Position());
    return m_Device->Write(data, size);
}

bool RangeBlockDevice::Seek(int rel, SeekPos pos) {
    if (!m_Device) return false;
    bool ok = true;
    switch (pos) {
        case SeekPos::Set:
            ok = m_Device->Seek(m_RangeBegin + rel, SeekPos::Set);
            break;
        case SeekPos::Current:
            ok = m_Device->Seek(rel, SeekPos::Current);
            break;
        case SeekPos::End:
            ok = m_Device->Seek(m_RangeBegin + m_RangeSize, SeekPos::End);
            break;
    }
    return ok;
}

size_t RangeBlockDevice::Position() {
    if (!m_Device) return -1;
    return m_Device->Position() - m_RangeBegin;
}

size_t RangeBlockDevice::Size() {
    return m_RangeSize;
}
