#pragma once
#include <stdint.h>
#include <string>
#include "MurmurHash3.h"

typedef uint32_t ResourceHash;
#define MURMUR_SEED 0xBEEFC0DE
namespace smug {
	static ResourceHash HashString(const std::string& s) {
		ResourceHash h;
		MurmurHash3_x86_32(s.data(), (int)s.length(), MURMUR_SEED, &h);
		return h;
	}
}