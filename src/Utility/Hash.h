#pragma once
#include <stdint.h>
#include <string>
#include <meow_intrinsics.h>
#include <meow_hash.h>

typedef uint32_t ResourceHash;
#define MEOW_SEED 0xBEEFC0DE
namespace smug {

	static ResourceHash HashString(const std::string& s) {
		meow_hash hash = MeowHash_Accelerated(MEOW_SEED, (int)s.length(), (void*)s.data());
		return MeowU32From(hash, 0);
	}
}