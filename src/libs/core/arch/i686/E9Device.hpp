#pragma once

#include <core/dev/CharacterDevice.hpp>

#include <stddef.h>
#include <stdint.h>

namespace arch {
    namespace i686 {
        class E9Device : public CharacterDevice { 
        public:
            virtual size_t Read(uint8_t* data, size_t size) override;
            virtual size_t Write(const uint8_t* data, size_t size) override;
        };
    }
}
