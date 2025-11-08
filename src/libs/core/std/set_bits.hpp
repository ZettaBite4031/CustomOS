#pragma once

#include <utility>

namespace std {
    template<typename T>
    T set_bits(T original, uint8_t start, uint8_t length, T value) {
        static_assert(std::is_unsigned<T>::value, "T must be unsigned (for bit safety)");

        if (length == 0 || start >= sizeof(T) * 8 || length > sizeof(T) * 8)
            return original; // No-op or invalid

        // Clamp to valid bit range
        if (start + length > sizeof(T) * 8)
            length = sizeof(T) * 8 - start;

        T mask = ((T{1} << length) - 1) << start;
        T shiftedValue = (value << start) & mask;

        return (original & ~mask) | shiftedValue;
    }
}