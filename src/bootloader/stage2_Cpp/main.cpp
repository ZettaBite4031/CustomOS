#include <core/ZosDefs.hpp>
#include <core/arch/i686/VGATextDevice.hpp>
#include <core/arch/i686/E9Device.hpp>
#include <core/dev/TextDevice.hpp>
#include <core/dev/RangeBlockDevice.hpp>
#include <core/fs/FATFileSystem.hpp>
#include <core/Debug.hpp>
#include <core/Assert.hpp>
#include <stdint.h>

#include "arch/i686/RealMemory.hpp"
#include "arch/i686/BIOSDisk.hpp"
#include "mem/Stage2Allocator.hpp"
#include "cpp/NewDelete.hpp"
#include "dev/MBR.hpp"
#include "memdefs.h"

Stage2Allocator g_Allocator(reinterpret_cast<void*>(MemoryMin), MemoryMax - MemoryMin);

arch::i686::VGATextDevice g_VGADevice;
arch::i686::E9Device g_DebugE9Device;

EXPORT void ASMCALL Start(uint16_t bootDrive, uint32_t partition_segoff) {
    SetCppAllocator(&g_Allocator);

    g_VGADevice.ClearScreen();

    TextDevice vgaTextDevice(&g_VGADevice);
    TextDevice e9TextDevice(&g_DebugE9Device);

    Debug::AddOutputDevice(&vgaTextDevice, Debug::DebugLevel::Info, false);
    Debug::AddOutputDevice(&e9TextDevice, Debug::DebugLevel::Debug);

    Debug::Info("Stage2", "Beginning Stage2 Load...");
    
    Debug::Info("Stage2", "Beginning initialization of disk #%d", bootDrive);
    BIOSDisk disk(bootDrive);
    if (!disk.Initialize()) {
        Debug::Error("Stage2", "Failed to initialize disk #%d", bootDrive);
        HALT
    }
    Debug::Info("Stage2", "Successfully initialized disk #%d", bootDrive);

    BlockDevice* partition;
    RangeBlockDevice partitionRange;
    if (bootDrive < 0x80)
        partition = &disk;
    else {
        Debug::Debug("Stage2", "Partition SegOff: %x", partition_segoff);
        Debug::Debug("Stage2", "Partition Linear: %x", ToLinear<void*>(partition_segoff));
        MBR_Entry* entry = ToLinear<MBR_Entry*>(partition_segoff);
        partitionRange.Initialize(&disk, entry->LBA_Start, entry->SectorCount);
        partition = &partitionRange;
        Debug::Debug("Stage2", "Partition Sector Count: %d", entry->SectorCount);
    }

    if (!partition) {
        Debug::Error("Stage2", "Failed to detect a partition!");
        HALT
    }
    Debug::Info("Stage2", "Found a partition!");

    FATFileSystem fs;
    if (!fs.Initialize(partition)) {
        Debug::Error("Stage2", "Failed to initialize the FAT file system!");
        HALT
    }
    Debug::Info("Stage2", "Successfully initialized the FAT file system!");

    File* kernel = fs.Open("kernel.elf", FileOpenMode::Read);

    HALT
}
