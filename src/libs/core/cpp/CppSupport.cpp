#include <core/ZosDefs.hpp>
#include <core/arch/i686/IO.hpp>

EXPORT int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle)
{
    return 0;
}

extern "C" [[noreturn]] void abort_kernel() { for(;;) { arch::i686::PANIC(); } } // or your panic()

namespace std {
    [[noreturn]] void __throw_length_error(const char*)             { abort_kernel(); }
    [[noreturn]] void __throw_bad_alloc()                           { abort_kernel(); }
    [[noreturn]] void __throw_bad_array_new_length()                { abort_kernel(); }
    [[noreturn]] void __throw_out_of_range(const char*)             { abort_kernel(); }
    [[noreturn]] void __throw_invalid_argument(const char*)         { abort_kernel(); }
    [[noreturn]] void __throw_logic_error(const char*)              { abort_kernel(); }
    [[noreturn]] void __throw_domain_error(const char*)             { abort_kernel(); }
    [[noreturn]] void __throw_overflow_error(const char*)           { abort_kernel(); }
    [[noreturn]] void __throw_underflow_error(const char*)          { abort_kernel(); }
    [[noreturn]] void __throw_runtime_error(const char*)            { abort_kernel(); }
}