#include "pio.h"

#include "io.h"
#include "timer.h"
#include <core/cpp/Memory.hpp>

static bool ATA_WaitDRQ() {
    for (int i = 0; i < 1000; i++) {
        uint8_t status = i686_InB(0x1F7);
        if (!(status & 0x80) && (status & 0x08)) return true;
        if (status & 0x01) return false;
        sleep(1);
    }

    return false;
}

bool ATA_ReadPIO(uint8_t drive, uint32_t lba, uint8_t count, void* buffer) {
    while (i686_InB(0x1F7) & 0x80) sleep(1);

    i686_OutB(0x1F6, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));

    i686_OutB(0x1F2, count);
    i686_OutB(0x1F3, (uint8_t)(lba & 0xFF));
    i686_OutB(0x1F4, (uint8_t)((lba >> 8) & 0xFF));
    i686_OutB(0x1F5, (uint8_t)((lba >> 16) & 0xFF));

    i686_OutB(0x1F7, 0x20);

    uint16_t* u16Buffer = (uint16_t*)buffer;
    for (uint8_t s = 0; s < count; s++) {
        if (!ATA_WaitDRQ()) return false;
        for (int i = 0; i < 256; i++) 
            u16Buffer[s*256 + i] = i686_InW(0x1F0);
    }

    return true;
}

#pragma region DMA mode

#define BMIDE_BASE      0x00C000
#define BMIDE_CMD       (BMIDE_BASE + 0x0) // R/W
#define BMIDE_STATUS    (BMIDE_BASE + 0x2) // R (write 1 to clear INT/ERR)
#define BMIDE_PRD_ADDR  (BMIDE_BASE + 0x4) // R/W (physical address of PRD table)

#define BM_CMD_START 0x01
#define BM_CMD_READ  0x08
#define BM_STAT_INT  0x04
#define BM_STAT_ERR  0x02

typedef struct prd_entry {
    uint32_t phys_base;
    uint32_t byte_count : 31;
    uint32_t end_of_table : 1;
} __attribute__((packed, aligned(0x10))) prd_entry_t;

static prd_entry_t prd_table[1];

bool ATA_ReadDMA(uint8_t drive, uint32_t lba, uint8_t count, void* buf) {
    prd_table[0].phys_base = (uint32_t)buf;
    prd_table[0].byte_count = count * 512;
    prd_table[0].end_of_table = 1;

    i686_OutL(BMIDE_PRD_ADDR, (uint32_t)prd_table);

    i686_OutB(BMIDE_STATUS, i686_InB(BMIDE_STATUS) & ~(BM_STAT_INT | BM_STAT_ERR));

    while (i686_InB(0x1F7) & 0x80) sleep(1);

    i686_OutB(0x1F6, 0xE0 | (drive << 4) | ((lba >> 24) & 0x0F));

    i686_OutB(0x1F2, count);
    i686_OutB(0x1F3, (uint8_t)(lba & 0xFF));
    i686_OutB(0x1F4, (uint8_t)((lba >> 8) & 0xFF));
    i686_OutB(0x1F5, (uint8_t)((lba >> 16) & 0xFF));

    i686_OutB(0x1F7, 0xC8);

    i686_OutB(BMIDE_CMD, BM_CMD_READ | BM_CMD_START);

    while (!(i686_InB(BMIDE_STATUS) & BM_STAT_INT)) sleep(1);

    i686_OutB(BMIDE_CMD, 0);

    uint8_t status = i686_InB(BMIDE_STATUS);
    if (status & BM_STAT_ERR) {
        i686_OutB(BMIDE_STATUS, status);
        return false;
    }

    i686_OutB(BMIDE_STATUS, status);

    return true;
}

#pragma endregion