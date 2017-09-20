#pragma once
#include <stdint.h>
#include <string>
#include "MurmurHash3.h"

typedef uint32_t ResourceHash;
#define MURMUR_SEED 0x7E57BEEF

ResourceHash HashString(const std::string& s) {
	ResourceHash h;
	MurmurHash3_x86_32(s.data(), s.length(), MURMUR_SEED, &h);
	return h;
}