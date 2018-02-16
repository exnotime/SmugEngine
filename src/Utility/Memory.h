#pragma once
namespace smug {
	static void* PointerAdd(void* p, size_t bytes) {
		return (void*)((char*)p + bytes);
	}
}

