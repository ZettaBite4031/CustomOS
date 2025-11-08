#include "Assert.hpp"

#include <arch/i686/IO.hpp>
#include <Debug.hpp>

bool _Assert(const char* condition, const char* file, int line, const char* func) {
    Debug::Critical("Assertion", "Assertion failed: `%s` !!!", condition);
    Debug::Critical("Assertion", "In '%s' on line %d (%s)", file, line, func);

    arch::i686::PANIC();
    return false;
}

void _Unreachable(const char* file, int line, const char* func) {
    Debug::Critical("Assertion - Unreachable", "Unreachable code!");
    Debug::Critical("Assertion - Unreachable", "In '%s' on line %d (%s)", file, line, func);

    arch::i686::PANIC();
}
void _Todo(const char* message, const char* file, int line, const char* func) {
    Debug::Critical("Assertion - TODO", "TODO: %s", message);
    Debug::Critical("Assertion - TODO", "In '%s' on line %d (%s)", file, line, func);

    arch::i686::PANIC();
}