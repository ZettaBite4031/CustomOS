#pragma once
#include <stdint.h>
#define FLAG_SET(n, f) ((n) |= (f))
#define FLAG_UNSET(n, f) ((n) &= ~(f));
