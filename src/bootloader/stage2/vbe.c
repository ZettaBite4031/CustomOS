#include "vbe.h"
#include "x86.h"
#include "memory.h"

bool VBE_GetControllerInfo(VbeInfoBlock* info) {
    if (x86_Video_GetVbeInfo(info) == 0x004F) {
        info->VideoModePtr = (uint32_t)SEGOFF2LIN((void*)info->VideoModePtr);
        return true;
    }
    return false;
}

bool VBE_GetModeInfo(uint16_t mode, VbeModeInfo* info) {
    return (x86_Video_GetModeInfo(mode, info) == 0x004F);
}

bool VBE_SetMode(uint16_t mode) {
    return x86_Video_SetMode(mode) == 0x004F;
}
