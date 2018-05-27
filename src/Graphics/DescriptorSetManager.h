#pragma once
#include <map>
#include <vulkan/vulkan.hpp>
namespace smug {
	static std::map<std::string, vk::DescriptorSet> g_DescSetMap;

	void RegisterDescriptorSet(const std::string& name, vk::DescriptorSet set) {
		auto& i = g_DescSetMap.find(name);
		if (i == g_DescSetMap.end()) {
			g_DescSetMap[name] = set;
		}
		else {
			//might want to update the set binding
		}
	}

	vk::DescriptorSet GetDescriptorSet(const std::string& name) {
		auto& i = g_DescSetMap.find(name);
		if (i != g_DescSetMap.end()) {
			return i->second;
		}
		return nullptr;
	}
}