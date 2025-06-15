#pragma once

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
        static_assert(!std::is_lvalue_reference<T>::value, "Forwarding an rvalue as an lvalue is not allowed!");
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
}


