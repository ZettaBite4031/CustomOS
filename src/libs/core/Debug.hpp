#pragma once

#include <core/dev/TextDevice.hpp>

namespace Debug {
    enum class DebugLevel : unsigned int {
        Debug = 0,
        Info = 1,
        Warn = 2,
        Error = 3,
        Critical = 4,
    };

    void Init();
    void AddOutputDevice(TextDevice* device, DebugLevel minLogLevel, bool useTextColor = true);

    void Debug(const char* module, const char* fmt, ...);
    void Info(const char* module, const char* fmt, ...);
    void Warn(const char* module, const char* fmt, ...);
    void Error(const char* module, const char* fmt, ...);
    void Critical(const char* module, const char* fmt, ...);
    void Raw(const char* fmt, ...);

    void HexDump(const char* msg, const void* bin, size_t size);
}