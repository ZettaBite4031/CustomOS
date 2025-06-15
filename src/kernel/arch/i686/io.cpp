#include "io.h"

#define UNUSED_PORT 0x80

void i686_IOWait() {
    i686_OutB(UNUSED_PORT, 0);
}