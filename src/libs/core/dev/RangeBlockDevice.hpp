#pragma once

#include "BlockDevice.hpp"

class RangeBlockDevice : public BlockDevice {
public:
    RangeBlockDevice();
    
    void Initialize(BlockDevice* device, size_t rangeBegin, size_t rangeSize);

    virtual size_t Read(uint8_t* data, size_t size) override;
    virtual size_t Write(const uint8_t* data, size_t size) override;
    virtual bool Seek(int rel, SeekPos pos) override;
    virtual size_t Position() override;
    virtual size_t Size() override;

private:
    BlockDevice* m_Device;
    size_t m_RangeBegin;
    size_t m_RangeSize;
};
