#include "debug.h"
#include "stdio.h"

static const char* const g_LogSeverityColors[] = {
    [LVL_DEBUG]     = "\033[2;37m",
    [LVL_INFO]      = "\033[37m",
    [LVL_WARN]      = "\033[1;33m",
    [LVL_ERROR]     = "\033[1;31m",
    [LVL_CRITICAL]  = "\033[1;37;41m",
};

static const char* const g_LogSeveritySymbols[] = {
    [LVL_DEBUG]     = "?",
    [LVL_INFO]      = "-",
    [LVL_WARN]      = "+",
    [LVL_ERROR]     = "*",
    [LVL_CRITICAL]  = "!",  
};

static const char* const g_DefaultColor = "\033[0m";

void logf(const char* module, DebugLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    if (level < MIN_LOG_LEVEL) return;

    fputs(g_LogSeverityColors[level], VFS_FD_DEBUG);                                // set color depending on level
    fprintf(VFS_FD_DEBUG, "[%s] [%s] - ", g_LogSeveritySymbols[level], module);     // print module
    vfprintf(VFS_FD_DEBUG, fmt, args);                                              // print message
    fputs(g_DefaultColor, VFS_FD_DEBUG);                                            // reset color
    fputc('\n', VFS_FD_DEBUG);                                                      // newline

    fprintf(VFS_FD_STDOUT, "[%s] [%s] - ", g_LogSeveritySymbols[level], module);     // print module
    vfprintf(VFS_FD_STDOUT, fmt, args);                                              // print message
    fputc('\n', VFS_FD_STDOUT);                                                      // newline

    va_end(args);
}
