#include "Debug.hpp"

#include <stdarg.h>
#include <core/dev/TextDevice.hpp>

namespace Debug {
    namespace {
        static inline const constexpr int MaximumLogDevices = 10;
        static inline constexpr const char* const g_DefaultColor = "\033[0m";
        static inline constexpr const char* const g_LogSeverityColors[] = { 
                    "\033[2;37m"    /* Debug */, 
                    "\033[37m"      /* Info */, 
                    "\033[1;33m"    /* Warn */, 
                    "\033[1;31m"    /* Error */, 
                    "\033[1;37;41m" /* Critical */ 
                };
        static inline constexpr const char* const g_LogSeveritySymbols[] = { 
                    "?" /* Debug */, 
                    "-" /* Info */, 
                    "+" /* Warn */, 
                    "*" /* Error */, 
                    "!" /* Critical */
                };

        struct {
            TextDevice* device;
            DebugLevel minLogLevel;
            bool useTextColor;
        } g_LogDevices[MaximumLogDevices];
        int g_LogDevicesCount;

        static void log(DebugLevel level, const char* module, const char* fmt, va_list args) { 
            for (int i = 0; i < g_LogDevicesCount; i++) {
                if (level < g_LogDevices[i].minLogLevel) continue;
                TextDevice* device = g_LogDevices[i].device;
                bool use_color = g_LogDevices[i].useTextColor;
                if (use_color) device->WriteF(g_LogSeverityColors[(int)level]);
                device->WriteF("[%s] [%s] - ", g_LogSeveritySymbols[(int)level], module);
                device->VWriteF(fmt, args);
                if (use_color) device->WriteF(g_DefaultColor);
                device->Write('\n');
            }
        }
    };

    void AddOutputDevice(TextDevice* device, DebugLevel minLogLevel, bool useTextColor) {
        if (g_LogDevicesCount + 1 > MaximumLogDevices) return;
        g_LogDevices[g_LogDevicesCount].device = device;
        g_LogDevices[g_LogDevicesCount].minLogLevel = minLogLevel;
        g_LogDevices[g_LogDevicesCount].useTextColor = useTextColor;
        g_LogDevicesCount++;
    }

    void Debug(const char* module, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log(DebugLevel::Debug, module, fmt, args);
        va_end(args);
    }

    void Info(const char* module, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log(DebugLevel::Info, module, fmt, args);
        va_end(args);
    }

    void Warn(const char* module, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log(DebugLevel::Warn, module, fmt, args);
        va_end(args);
    }
    
    void Error(const char* module, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log(DebugLevel::Error, module, fmt, args);
        va_end(args);
    }
    
    void Critical(const char* module, const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        log(DebugLevel::Critical, module, fmt, args);
        va_end(args);
    }

}