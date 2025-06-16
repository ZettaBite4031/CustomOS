#pragma once

#include <stdint.h>
#include <stddef.h>

namespace PIT {
    void Init(uint32_t frequency);
}

void sleep(uint32_t ms);
