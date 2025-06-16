#pragma once

#include <stdint.h>
#include <core/dev/BlockDevice.hpp>
#include "IOAllocator.hpp"

struct ATAIdentifyDevice {
    uint16_t GeneralConfig;               // Word 0
    uint16_t _1[9];                 // Words 1–9
    uint8_t  SerialNumber[20];           // Words 10–19 (swap bytes per word)
    uint16_t _2[3];                // Words 20–22
    uint8_t  FirmwareRevision[8];        // Words 23–26
    uint8_t  ModelNumber[40];            // Words 27–46
    uint8_t  _3[2];            // Word 47
    uint16_t TrustedComputing;           // Word 48
    uint16_t Capabilities;                // Word 49
    uint16_t Capabilities2;               // Word 50
    uint16_t _4[2];                // Words 51–52
    uint16_t FieldValidity;              // Word 53
    uint16_t _5[5];                // Words 54–58
    uint16_t MultiSectorSetting;        // Word 59
    uint32_t LBA28SectorCount;          // Words 60–61
    uint16_t _6;                   // Word 62
    uint16_t DMAModes;                   // Word 63
    uint16_t PIOModes;                   // Word 64
    uint16_t _7[5];                // Words 65–69
    uint16_t QueueDepth;                 // Word 75
    uint16_t SATACapabilities;           // Word 76
    uint16_t SATAAdditionalCapabilities;// Word 77
    uint16_t SATAFeaturesSupported;     // Word 78
    uint16_t SATAFeaturesEnabled;       // Word 79
    uint16_t ATAMajorVersion;           // Word 80
    uint16_t ATAMinorVersion;           // Word 81
    uint16_t CommandSet_Supported;       // Word 82
    uint16_t CommandSet_Supported2;      // Word 83
    uint16_t CommandSet_Supported3;      // Word 84
    uint16_t CommandSet_Enabled;         // Word 85
    uint16_t CommandSet_Enabled2;        // Word 86
    uint16_t CommandSet_Enabled3;        // Word 87
    uint16_t UltaDMAModes;             // Word 88
    uint16_t _8[5];           // Words 89–93
    uint16_t HardwareResetResult;       // Word 93
    uint16_t _9[6];           // Words 94–99

    // LBA48 support
    uint64_t LBA48SectorCount;          // Words 100–103

    uint32_t MaxLBA48PerCommand;       // Words 104–105
    uint16_t LogicalSectorSizeInfo;    // Word 106
    uint16_t _10[10];          // Words 107–116

    uint32_t LogicalSectorSizeBytes;   // Words 117–118
    uint16_t _11[137];         // Words 119–255 (vendor/reserved)
} PACKED;

class Disk : public BlockDevice {
public:
    Disk(uint32_t drive_id, IORange* range, bool ranged = false);

    virtual size_t Read(uint8_t* data, size_t size) override;
    virtual size_t Write(const uint8_t* data, size_t size) override;
    virtual bool Seek(int rel, SeekPos pos) override;
    virtual size_t Position() override;
    virtual size_t Size() override;

    bool Initialize();

    static constexpr uint32_t BytesPerSector{ 512 }; 
private:
    
    bool WaitDRQ();
    bool ReadNextSector();
    bool WriteCurrentSector();

    ATAIdentifyDevice m_Configuration;

    IORange* m_Range{ nullptr };
    uint32_t m_DriveID{ static_cast<uint32_t>(-1) };
    uint32_t m_CurrentLBA{ static_cast<uint32_t>(-1) };

    uint8_t m_Buffer[BytesPerSector];
    uint32_t m_Position{ 0 };
    size_t m_Size{ 0 };
    bool m_Initialized{ false };
    bool m_UsedByRangedDevice{ false };
};