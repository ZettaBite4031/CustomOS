#pragma once
#include <core/ZosDefs.hpp>
#include <stdint.h>
#include <stddef.h>

bool islower(char c);
char toupper(char c);

#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_UNSET(n, f) ((n) &= ~(f));

EXPORT const char* strchr(const char* str, char chr);
EXPORT char* strcpy(char* dst, const char* src);
EXPORT char* strncpy(char* dst, const char* src, size_t n);
EXPORT unsigned strlen(const char* str);
EXPORT size_t strnlen(const char* str, size_t n);
EXPORT int strcmp(const char* a, const char* b);
EXPORT int strncmp(const char* a, const char* b, size_t n);

char* strdup(const char* src);
char* strndup(const char* src, size_t size);

namespace String {
    constexpr auto Find = strchr;
    constexpr auto Copy = strcpy;
    constexpr auto Length = strlen;
    constexpr auto Compare = strcmp;
}
