#include "E9Device.hpp"
#include "IO.hpp"

namespace arch {
    namespace i686 {
        size_t E9Device::Read(uint8_t* data, size_t size) {
            return -1;
        }

        size_t E9Device::Write(const uint8_t* data, size_t size) {
            for (int i = 0; i < size; i++) 
                OutPortB(0xE9, data[i]);
            return size;
        }
    }
}
