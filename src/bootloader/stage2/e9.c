#include "e9.h"
#include "x86.h"

void E9_putc(char c) {
    x86_OutB(0xE9, c);
}
