#pragma once
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
//int Vsnprintf8(char* pDestination, size_t n, const char* pFormat, va_list arguments);
//int Vsnprintf16(char16_t* pDestination, size_t n, const char16_t* pFormat, va_list arguments);