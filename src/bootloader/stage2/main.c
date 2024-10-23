#include <stdint.h>
#include "stdio.h"
#include "disk.h"
#include "mbr.h"
#include "fat.h"
#include "memdefs.h"
#include "memory.h"
#include "vbe.h"
#include <stddef.h>
#include "zosdefs.h"
#include "debug.h"
#include "elf.h"

#define COLOR(r,g,b) ((b) | (g << 8) | (r << 16))

uint8_t* KernelLoadBuffer = (uint8_t*)MEMORY_LOAD_KERNEL_ADDR;
uint8_t* Kernel = (uint8_t*)MEMORY_KERNEL_ADDR;

typedef void(*KernelMain)();

void __attribute__((cdecl)) Start(uint16_t bootDrive, void* partition_location) {
    cls();
    puts("=-=-=-=-= ZOS STAGE2 LOADING =-=-=-=-=\n");

    LogInfo("Stage2 Main", "Beginning Stage 2 load...");

    DISK disk;
    LogInfo("Stage2 Main", "Beginning Disk initialization...");
    if (!DISK_Initialize(&disk, bootDrive)) {
        LogCritical("Stage2 Main", "Failed to initialize boot drive %d!!", bootDrive);
        printf("DISK: Failed to initialize boot drive!\n");
        goto end;
    }

    Partition partition;
    MBR_DetectPartition(&partition, &disk, partition_location);
    LogInfo("Stage2 Main", "Detected Partition! Disk ID: 0x%x", partition.disk->id);

    LogInfo("Stage2 Main", "Initialization FAT driver...");
    if (!FAT_Initialize(&partition)) {
        LogCritical("Stage2 Main", "Failed to initialize the FAT Driver!!");
        printf("FAT: Failed to initialize!\n");
        goto end;
    }

    // list root directory
    FAT_File* fd = FAT_Open(&partition, "/");
    FAT_DirectoryEntry entry;
    int i = 0; 
    while (FAT_ReadEntry(&partition, fd, &entry) && i++ < 5) {
        printf("  ");
        for (int i = 0; i < 11; i++) putc(entry.Name[i]);
        printf("\r\n");
    }
    FAT_Close(fd);

    // load kernel
    void* kernelEntryPoint;
    LogInfo("Stage2 Main", "Loading the Kernel ELF...");
    if(!ELF_Read(&partition, "/boot/kernel.elf", &kernelEntryPoint)) {
        LogCritical("Stage2 Main", "Failed to load the kernel!!");
        printf("Failed to load the kernel!!\r\n");
        goto end;
    }

    // execute kernel
    LogInfo("Stage2 Main", "Kernel loaded successfully. Beginning execution...")
    KernelMain kernelEntry = (KernelMain)kernelEntryPoint;
    kernelEntry();

    LogError("Stage2 Main", "The kernel has broken out of confinement!");
    cls();
    
    #pragma region Graphics
graphics:
    // Test Graphics
    const int desiredWidth = 1024;
    const int desiredHeight = 768;
    const int desiredBPP = 32;
    uint16_t chosenMode = 0xFFFF;

    VbeInfoBlock* info = (VbeInfoBlock*)MEMORY_VESA_INFO;
    VbeModeInfo* modeInfo = (VbeModeInfo*)MEMORY_VIDEO_MODE_INFO;
    if (VBE_GetControllerInfo(info)) {
        uint16_t* mode = (uint16_t*)(info->VideoModePtr);
        for (int i = 0; mode[i] != 0xFFFF; i++) {
            if (!VBE_GetModeInfo(mode[i], modeInfo)) {
                printf("Could not retrieve info for mode %x\n", mode[i]);
                continue;
            }
            bool hasFB = (modeInfo->attributes & 0x90) == 0x90;
            if (hasFB && modeInfo->width == desiredWidth && modeInfo->height == desiredHeight && modeInfo->bpp == desiredBPP) {
                chosenMode = mode[i];
                break;
            }
        }

        if (chosenMode != 0xFFFF && VBE_SetMode(chosenMode)) {
            uint32_t* FrameBuffer = (uint32_t*)(modeInfo->framebuffer);
            int w = modeInfo->width;
            int h = modeInfo->height;
            for (int y = 0; y < h; y++) 
                for (int x = 0; x < w; x++) 
                    FrameBuffer[y * modeInfo->pitch / 4 + x] = COLOR(x, y, (x+y)/2);
        }
    } 
    else printf("No VBE extensions :<\n");
    #pragma endregion
end:
    for(;;);
}
