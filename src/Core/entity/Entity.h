#pragma once
#include <stdint.h>
#include <vector>
#include <intrin.h>
#define MAX_COMPONENTS 64
namespace smug {
	struct Entity {
		uint32_t UID = 0;
		uint32_t Tag = 0;
		uint64_t ComponentBitfield = 0;
		std::vector<uint32_t> Components;
	};

	static uint32_t fast_log2(uint64_t t) {
		unsigned long bit_index = 0;
		if (_BitScanForward64(&bit_index, t)) {
			return bit_index;
		}
		return 0;
	}

	//assumes we have already checked that the entity has the component
	static uint32_t GetEntityComponentIndex(const Entity& e, const uint64_t componentFlag) {
		uint32_t i = fast_log2(componentFlag);
		uint64_t bits = i != 0 ? e.ComponentBitfield << (MAX_COMPONENTS - i) : 0;
		return e.Components[__popcnt64(bits)];
	}

	static void RemoveComponentfromEntity(Entity& e, uint64_t componentFlag) {
		uint32_t i = fast_log2(componentFlag);
		uint64_t t = i != 0 ? (e.ComponentBitfield << (MAX_COMPONENTS - i)) : 0;
		uint64_t index = __popcnt64(t);
		e.ComponentBitfield &= ~componentFlag;
		e.Components.erase(e.Components.begin() + index);
	}

	static void AddComponentToEntity(Entity& e, uint64_t componentFlag, uint32_t componentIndex) {
		uint32_t i = fast_log2(componentFlag);
		uint64_t t = i != 0 ? (e.ComponentBitfield << (MAX_COMPONENTS - i)) : 0;
		uint64_t index = __popcnt64(t);
		e.ComponentBitfield |= componentFlag;
		e.Components.insert(e.Components.begin() + index, componentIndex);
	}
}