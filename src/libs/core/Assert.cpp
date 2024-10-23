#include "Assert.hpp"

#include <arch/i686/IO.hpp>
#include <Debug.hpp>

bool _Assert(const char* condition, const char* file, int line, const char* func) {
    Debug::Critical("Assert", "Assertion failed: `%s` !!!", condition);
    Debug::Critical("Assert", "In '%s' on line %d (%s)", file, line, func);

    arch::i686::PANIC();
    return false;
}