#include <stdio.h>

typedef enum {
    LVL_DEBUG = 0,
    LVL_INFO = 1,
    LVL_WARN = 2,
    LVL_ERROR = 3,
    LVL_CRITICAL = 4,
} DebugLevel;

// Logging function which uses the "E9 hack" in Qemu to log to terminal - Generic
void logf(const char* module, DebugLevel level, const char* fmt, ...);

#define MIN_LOG_LEVEL LVL_DEBUG

// Logging macro which uses the "E9 hack" in Qemu to log to terminal - Debug Level
#define LogDebug(module, ...) logf(module, LVL_DEBUG, __VA_ARGS__);

// Logging macro which uses the "E9 hack" in Qemu to log to terminal - Info Level
#define LogInfo(module, ...) logf(module, LVL_INFO, __VA_ARGS__);

// Logging macro which uses the "E9 hack" in Qemu to log to terminal - Warn level
#define LogWarn(module, ...) logf(module, LVL_WARN, __VA_ARGS__);

// Logging macro which uses the "E9 hack" in Qemu to log to terminal - Error Level
#define LogError(module, ...) logf(module, LVL_ERROR, __VA_ARGS__);

// Logging macro which uses the "E9 hack" in Qemu to log to terminal - Critical Failure Level
#define LogCritical(module, ...) logf(module, LVL_CRITICAL, __VA_ARGS__);
