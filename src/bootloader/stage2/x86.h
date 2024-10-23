#pragma once
#include <stdint.h>
#include <stdbool.h>

void __attribute__((cdecl)) x86_OutB(uint16_t port, uint8_t value);
uint8_t __attribute__((cdecl)) x86_InB(uint16_t port);

bool __attribute__((cdecl)) x86_DiskReset(uint8_t drive);
bool __attribute__((cdecl)) x86_DiskRead(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t* lowerDataOut);
bool __attribute__((cdecl)) x86_GetDiskDriveParams(uint8_t drive, uint8_t* driveType, uint16_t* cylinders, uint16_t* sectors, uint16_t* heads);

int __attribute__((cdecl)) x86_Video_GetVbeInfo(void* infoOut);
int __attribute__((cdecl)) x86_Video_GetModeInfo(uint16_t mode, void* infoOut);
int __attribute__((cdecl)) x86_Video_SetMode(uint16_t mode);
