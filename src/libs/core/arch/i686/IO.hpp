#pragma once

#include <stdint.h>

#include <core/ZosDefs.hpp>

namespace arch {
    namespace i686 {
        EXPORT void ASMCALL OutPort(uint16_t port, uint8_t value);
        EXPORT uint8_t ASMCALL InPort(uint16_t port);

        EXPORT void ASMCALL PANIC();
    }
}
