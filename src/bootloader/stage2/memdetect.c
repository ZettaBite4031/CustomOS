#include "memdetect.h"

#include "x86.h"
#include "debug.h"

MemoryRegion g_MemRegions[MAX_REGIONS];
int g_MemRegionCount;


int Memory_Detect(MemoryInfo* memoryInfo) {
    E820MemoryBlock block;
    uint32_t continuation = 0;
    int ret;

    g_MemRegionCount = 0;
    ret = x86_E820GetNextBlock(&block, &continuation);

    while (ret > 0 && continuation != 0) {

        g_MemRegions[g_MemRegionCount].Begin = block.Base;
        g_MemRegions[g_MemRegionCount].Length = block.Length;
        g_MemRegions[g_MemRegionCount].Type = block.Type;
        g_MemRegions[g_MemRegionCount].ACPI = block.ACPI;
        g_MemRegionCount++;

        LogInfo("Memory Detect", "Block %d | Base 0x%llx | Length 0x%llx | Type 0x%x | ACPI 0x%x", g_MemRegionCount, block.Base, block.Length, block.Type, block.ACPI);

        ret = x86_E820GetNextBlock(&block, &continuation);
    }

    memoryInfo->BlockCount = g_MemRegionCount;
    memoryInfo->Regions = g_MemRegions;

    return ret;
}
