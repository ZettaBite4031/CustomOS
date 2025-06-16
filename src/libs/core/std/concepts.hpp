#pragma once

#include "Utility.hpp"

namespace std {
    template<typename T>
    struct is_unsigned { static constexpr bool value = false; };

    template<> struct is_unsigned<unsigned char>        { static constexpr bool value = true; };
    template<> struct is_unsigned<unsigned short>       { static constexpr bool value = true; };
    template<> struct is_unsigned<unsigned int>         { static constexpr bool value = true; };
    template<> struct is_unsigned<unsigned long>        { static constexpr bool value = true; };
    template<> struct is_unsigned<unsigned long long>   { static constexpr bool value = true; };


    template<typename T>
    constexpr bool is_unsigned_v = is_unsigned<T>::value;

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