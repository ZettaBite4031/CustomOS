#pragma once

#include <stdint.h>
#include <core/dev/BlockDevice.hpp>

static inline constexpr int BIOSSectorSize = 512;

class BIOSDisk : public BlockDevice {
public:
    BIOSDisk(uint8_t id); 

    virtual size_t Read(uint8_t* data, size_t size) override;        
    virtual size_t Write(const uint8_t* data, size_t size) override; 
    virtual bool Seek(int rel, SeekPos pos) override;
    virtual size_t Position() override;
    virtual size_t Size() override;
    
    bool Initialize();

private:
    bool ReadNextSector();
    void LBA2CHS(uint32_t LBA, uint16_t* cyl, uint16_t* sec, uint16_t* head);

    uint8_t m_ID;
    uint8_t m_DriveType;
    bool m_HasExtensions;
    uint16_t m_Cylinders;
    uint16_t m_Sectors;
    uint16_t m_Heads;

    uint8_t m_Buffer[BIOSSectorSize];
    uint32_t m_Position;
    size_t m_Size;
    
};
