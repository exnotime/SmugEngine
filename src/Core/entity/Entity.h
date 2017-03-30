#pragma once
#include <vector>
struct Entity {
	unsigned int UID = 0;
	unsigned int ComponentBitfield = 0;
	unsigned int Tag = 0;
	std::vector<int> Components;
};