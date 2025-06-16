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

        static void DumpHex(const char* msg, const void* bin, size_t size) {
            const uint8_t* u8buf = static_cast<const uint8_t*>(bin);
            if (msg) Debug::Info("HexDump", "%s", msg);
            for (size_t i{ 0 }; i < size; i+= 8) {
                Debug::Raw("%s[X] [HexDump] - ", g_LogSeverityColors[(int)DebugLevel::Info]);
                Debug::Raw("%08x |", static_cast<unsigned int>(i));

                for (size_t j{ 0 }; j < 8; j++) {
                    if (i + j < size) Debug::Raw(" %02x", u8buf[i + j]);
                    else Debug::Raw("   ");
                    if (j % 4 == 3 && j % 4 != 0) Debug::Raw(" ");
                }

                Debug::Raw(" | ");
                for (size_t j = 0; j < 8; ++j) {
                    if (i + j < size) {
                        uint8_t c = u8buf[i + j];
                        Debug::Raw("%c", (c >= 32 && c <= 126) ? c : '.');
                    }
                }

                Debug::Raw(g_DefaultColor);
                Debug::Raw("\n");
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

    void Raw(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        for (int i = 0; i < g_LogDevicesCount; i++) {
            g_LogDevices[i].device->VWriteF(fmt, args);
        }
        va_end(args);
    }

    void HexDump(const char* msg, const void* bin, size_t size) {
        DumpHex(msg, bin, size);
    }

}