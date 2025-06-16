#pragma once

#include <stdint.h>
#include <stddef.h>

#if !defined(__i386__) && !defined(__x86_64__)
#include <arch/i686/IO.hpp>
#endif

template<typename T>
class Atomic {
    static_assert(__is_trivially_copyable(T), "Atomic only supports trivially copyable types!");

private:
    volatile T value;

public:
    constexpr Atomic() noexcept = default;
    constexpr Atomic(T initial) noexcept : value(initial) {}

    T Load() const noexcept {
        return value;
    }

    void Store(T v) noexcept {
        value = v;
    }

    T Exchange(T newVal) noexcept {
        T old;
    #if defined(__i386__) || defined(__x86_64__) 
        __asm__ __volatile__(
            "xchg %[newval] %[oldval]"
            : [oldval] "=m"(value), "+r"(newVal)
            : [newval] "1"(newVal)
            : "memory");
        old = newVal;
    #else
        DisableInterrupts();
        old = value;
        value = newVal;
        EnableInterrupts();
    #endif
        return old;
    }   

    bool CompareExchange(T& expected, T desired) noexcept {
        bool result;
    #if defined(__i386__) || defined(__x86_64__) 
        result = __sync_bool_compare_and_swap(&value, expected, desired);
    #else
        DisableInterrupts();
        if (value == expected) {
            value = desired;
            result = true;
        } else {
            expected = value;
            result = false;
        }
        EnableInterrupts();
    #endif
        return result;
    }

    T FetchAdd(T delta) noexcept {
    #if defined(__i386__) || defined(__x86_64__) 
        return __sync_fetch_and_add(&value, delta);
    #else
        DisableInterrupts();
        T old = value;
        value += delta;
        EnableInterrupts();
        return old;
    #endif
    }

    T FetchSub(T delta) noexcept {
    #if defined(__i386__) || defined(__x86_64__) 
        return __sync_fetch_and_add(&value, -delta);
    #else
        DisableInterrupts();
        T old = value;
        value -= delta;
        EnableInterrupts();
        return old;
    #endif
    }

    T operator++() { return FetchAdd(1) + 1; }
    T operator--() { return FetchAdd(-1) - 1; }
    operator T() const { return Load(); }
};