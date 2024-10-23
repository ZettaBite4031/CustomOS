#include "utility.h"

uint32_t aling(uint32_t num, uint32_t alignto) {
    if (alignto == 0) return num;
    uint32_t rem = num % alignto;
    return (rem > 0) ? (num + alignto - rem) : num;
}
