#pragma once

#define assert(cond) Assert(cond)
#define Assert(condition) ((condition) || _Assert(#condition, __FILE__, __LINE__, __FUNCTION__))

#define unreachable() Unreachable()
#define Unreachable() _Unreachable(__FILE__, __LINE__, __FUNCTION__)

#define todo(msg) Todo(msg) 
#define Todo(message) _Todo(message, __FILE__, __LINE__, __FUNCTION__)  

bool _Assert(const char* condition, const char* file, int line, const char* func);
void _Unreachable(const char* file, int line, const char* func);
void _Todo(const char* message, const char* file, int line, const char* func);