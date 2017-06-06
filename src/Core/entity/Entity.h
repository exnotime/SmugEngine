#pragma once
#include <vector>
struct Entity {
	uint64_t UID = 0;
	uint32_t ComponentBitfield = 0;
	uint32_t Tag = 0;
	std::vector<uint32_t> Components;
};