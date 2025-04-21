#include "qemu.h"
#include <arch/i686/io.h>


void exit(uint8_t c) {
    i686_OutB(0xF4, c);
}