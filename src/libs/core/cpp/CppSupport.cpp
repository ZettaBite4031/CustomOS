#include <core/ZosDefs.hpp>
#include <core/arch/i686/IO.hpp>

EXPORT int __cxa_atexit(void (*destructor) (void *), void *arg, void *__dso_handle)
{
    return 0;
}
