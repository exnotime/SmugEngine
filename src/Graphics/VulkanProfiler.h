#pragma once
#include "volk.h"
#include "DeviceAllocator.h"
#include <string>
namespace smug {
	class VulkanProfiler {
	public:
		VulkanProfiler() {}
		~VulkanProfiler() {}

		void Init(VkDevice device, VkPhysicalDevice gpu);
		void Reset(VkDevice device);
		void Stamp(VkCommandBuffer cmdBuffer, const char* name);
		void Print();

	private:
		struct TimeStamp {
			std::string name;
			uint64_t offset;
			uint64_t timeStamp;
		};
		std::vector<TimeStamp> m_TimeStamps[BUFFER_COUNT];
		uint32_t m_QueryOffset[BUFFER_COUNT];

		VkQueryPool m_Pool[BUFFER_COUNT];
		uint32_t m_FrameIndex;
		float m_TimeStampPeriod;
	};
}