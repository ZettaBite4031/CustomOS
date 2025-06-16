#include "System.hpp"

#include <core/arch/i686/IO.hpp>

void exit(uint8_t code) {
    arch::i686::OutPortB(0xF4, code);
}