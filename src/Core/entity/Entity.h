#pragma once
#include <stdint.h>
#define MAX_COMPONENTS 64

struct Entity {
	uint64_t UID = 0;
	uint64_t ComponentBitfield = 0;
	uint32_t Tag = 0;
	uint32_t Components[MAX_COMPONENTS];
};