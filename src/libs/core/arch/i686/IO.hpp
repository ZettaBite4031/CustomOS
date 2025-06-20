#pragma once

#include <stdint.h>

#include <core/ZosDefs.hpp>

namespace arch {
    namespace i686 {
        EXPORT void ASMCALL OutPortB(uint16_t port, uint8_t value);
        EXPORT uint8_t ASMCALL InPortB(uint16_t port);
        EXPORT void ASMCALL OutPortW(uint16_t port, uint16_t value);
        EXPORT uint16_t ASMCALL InPortW(uint16_t port);
        EXPORT void ASMCALL OutPortL(uint16_t port, uint32_t value);
        EXPORT uint32_t ASMCALL InPortL(uint16_t port);
        
        EXPORT void ASMCALL DisableInterrupts();
        EXPORT void ASMCALL EnableInterrupts();

        EXPORT void ASMCALL PANIC();

        EXPORT void ASMCALL LoadGDT(void* desc, uint16_t cs, uint16_t ds);
        EXPORT void ASMCALL LoadIDT(void* desc);

        EXPORT void ASMCALL IOWait();
    }
}
