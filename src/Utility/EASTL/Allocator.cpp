#pragma once
#include "Allocator.h"
#include <stdlib.h>
#include <assert.h>
void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
	return malloc(size);
}
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line) {
	assert(alignment <= 8);
	return malloc(size);
}