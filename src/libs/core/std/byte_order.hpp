#pragma once

#include <cstddef>

namespace std {
    constexpr inline static uint16_t ntohs(uint16_t v) {
        uint16_t res = 0;
        for (std::size_t i = 0; i < sizeof(v); i++) {
            res |= ((v >> (i * 8)) & static_cast<uint16_t>(0xFF)) << ((sizeof(v) - 1 - i) * 8);
        }
        return res;
    }

    constexpr inline static uint16_t htons(uint16_t v) {
        return ntohs(v);
    }

    constexpr inline static uint32_t ntohl(uint32_t v) {
        uint32_t res = 0;
        for (std::size_t i = 0; i < sizeof(v); i++) {
            res |= ((v >> (i * 8)) & static_cast<uint32_t>(0xFF)) << ((sizeof(v) - 1 - i) * 8);
        }
        return res;
    }

    constexpr inline static uint32_t htonl(uint16_t v) {
        return ntohl(v);
    }
}