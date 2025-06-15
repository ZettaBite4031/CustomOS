#pragma once

#include "Utility.hpp"
namespace std {
    template<typename T>
    struct is_unsigned : std::false_type {};

    template<> struct is_unsigned<unsigned char>  : std::true_type {};
    template<> struct is_unsigned<unsigned short> : std::true_type {};
    template<> struct is_unsigned<unsigned int>   : std::true_type {};
    template<> struct is_unsigned<unsigned long>  : std::true_type {};
    template<> struct is_unsigned<unsigned long long> : std::true_type {};


    template<typename T>
    constexpr bool is_unsigned_v = is_unsigned<T>::value;
}