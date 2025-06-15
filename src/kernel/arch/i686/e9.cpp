#include "e9.h"
#include <arch/i686/io.h>

void E9_putc(char c) {
    i686_OutB(0xE9, c);
}
