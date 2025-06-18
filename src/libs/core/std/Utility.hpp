#pragma once

#include <core/std/concepts.hpp>

namespace std {

    template<typename T> struct remove_reference        { using type = T; };
    template<typename T> struct remove_reference<T&>    { using type = T; };
    template<typename T> struct remove_reference<T&&>   { using type = T; };

    template<typename T> struct is_lvalue_reference     { static constexpr bool value = false; };
    template<typename T> struct is_lvalue_reference<T&> { static constexpr bool value = true; };

    template<typename T>
    constexpr typename std::remove_reference<T>::type&& move (T&& t) noexcept {
        return static_cast<typename std::remove_reference<T>::type&&>(t);
    }

    template<typename T>
    constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept {
        //static_assert(!std::is_lvalue_reference<T>::value, "Forwarding an rvalue as an lvalue is not allowed!");
        return static_cast<T&&>(t);
    }

    template<typename T>
    T* addressof(T& t) noexcept {
        return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(t)));
    }

    struct true_type { static constexpr bool value = true; };
    struct false_type { static constexpr bool value = false; };

    template<typename T>
    T&& declval() noexcept;

    template<typename T>
    struct is_default_constructible {
    private:
        template<typename U, typename = decltype(U())>
        static true_type test(int);

        template<typename>
        static false_type test(...);

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<typename T>
    struct is_copy_constructible {
    private:
        template<typename U, typename = decltype(U(declval<const U&>()))>
        static true_type test(int);

        template<typename>
        static false_type test(...);

    public:
        static constexpr bool value = decltype(test<T>(0))::value;
    };

    template<bool Condition, typename T = void>
    struct enable_if { };

    template<typename T>
    struct enable_if<true, T> {
        using type = T;
    };

    template<bool Condition, typename T = void>
    using enable_if_t = typename enable_if<Condition, T>::type;

    template<typename T, typename U>
    struct is_same {
        static constexpr bool value = false;
    };

    template<typename T>
    struct is_same<T, T> {
        static constexpr bool value = true;
    };

    template<typename T, typename U>
    constexpr bool is_same_v = is_same<T, U>::value;

    template<typename T1, typename T2> 
    struct pair {
        T1 first;
        T2 second;

        //constexpr pair() = default;
        constexpr pair(const T1& a, const T2& b) 
            : first(a), second(b) {}

        template<typename U1, typename U2>
        constexpr pair(U1&& a, U2&& b) 
            : first(static_cast<U1&&>(a)), second(static_cast<U2&&>(b)) {}
    };

    template<typename T, T v>
    struct integral_constant {
        static constexpr T value = v;
        using value_type = T;
        using type       = integral_constant;   // handy alias

        constexpr operator value_type() const noexcept { return value; }

        [[nodiscard]] constexpr value_type operator()() const noexcept { return value; }
    };


    template<size_t I, typename T1, typename T2>
    constexpr auto& get(pair<T1, T2>& p) noexcept {
        if constexpr (I == 0) return p.first;
        else static_assert(I == 1, "pair only has two elements");
        return p.second;
    }

    template<size_t I, typename T1, typename T2>
    constexpr const auto& get(const pair<T1, T2>& p) noexcept {
        if constexpr (I == 0) return p.first;
        else static_assert(I == 1, "pair only has two elements");
        return p.second;
    }

    template<typename Container>
    auto begin(Container& c) -> decltype(c.begin()) {
        return c.begin();
    }

    template<typename Container>
    auto end(Container& c) -> decltype(c.end()) {
        return c.end();
    }
    
    template<typename T>
    constexpr T htons(T v) {
        static_assert(std::is_unsigned<T>::value, "htons only supports unsigned types!");

        T res = 0;
        for (size_t i{ 0 }; i < sizeof(T); i++) {
            res |= ((v >> (i * 8)) & static_cast<T>(0xFF)) << ((sizeof(T) - 1 - i) * 8); 
        }
        return res;
    }

    template<typename T>
    constexpr T ntohs(T v) {
        return htons(v);
    }
}


