#pragma once

#include <stdint.h>
#include <stdbool.h>

bool ATA_ReadPIO(uint8_t drive, uint32_t lba, uint8_t count, void* buffer);
//bool ATA_ReadDMA(uint8_t drive, uint32_t lba, uint8_t count, void* buf);