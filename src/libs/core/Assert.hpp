#pragma once

#define assert(cond) Assert(cond)
#define Assert(condition) ((condition) || _Assert(#condition, __FILE__, __LINE__, __FUNCTION__))

bool _Assert(const char* condition, const char* file, int line, const char* func);
