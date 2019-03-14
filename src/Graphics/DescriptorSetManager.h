#pragma once
#include <map>
#include "volk.h"
namespace smug {
static std::map<std::string, VkDescriptorSet> g_DescSetMap;

void RegisterDescriptorSet(const std::string& name, VkDescriptorSet set) {
	auto& i = g_DescSetMap.find(name);
	if (i == g_DescSetMap.end()) {
		g_DescSetMap[name] = set;
	} else {
		//might want to update the set binding
	}
}

VkDescriptorSet GetDescriptorSet(const std::string& name) {
	auto& i = g_DescSetMap.find(name);
	if (i != g_DescSetMap.end()) {
		return i->second;
	}
	return nullptr;
}
}