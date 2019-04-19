#include "VulkanProfiler.h"
#include "Imgui/imgui.h"
using namespace smug;

void VulkanProfiler::Init(VkDevice device, VkPhysicalDevice gpu) {
	VkQueryPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.queryType = VkQueryType::VK_QUERY_TYPE_TIMESTAMP;
	poolInfo.queryCount = 100000;
	for (uint32_t i = 0; i < BUFFER_COUNT; ++i) {
		vkCreateQueryPool(device, &poolInfo, nullptr, &m_Pool[i]);
		m_QueryOffset[i] = 0;
	}
	m_FrameIndex = 0;
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(gpu, &props);
	m_TimeStampPeriod = props.limits.timestampPeriod;
}

void VulkanProfiler::Reset(VkDevice device) {
	m_FrameIndex = (m_FrameIndex + 1) % BUFFER_COUNT;

	uint32_t count = m_QueryOffset[m_FrameIndex];
	if (count == 0)
		return;
	eastl::vector<uint64_t> timeStamps;
	timeStamps.resize(count);
	vkGetQueryPoolResults(device, m_Pool[m_FrameIndex], 0, count, sizeof(uint64_t) * count, timeStamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
	
	for (auto& ts : m_TimeStamps[m_FrameIndex]) {
		ts.timeStamp = timeStamps[ts.offset];
	}
	m_QueryOffset[m_FrameIndex] = 0;

}

void VulkanProfiler::Stamp(VkCommandBuffer cmdBuffer, const char* name) {
	vkCmdWriteTimestamp(cmdBuffer, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_Pool[m_FrameIndex], m_QueryOffset[m_FrameIndex]);
	TimeStamp ts;
	ts.name = name;
	ts.offset = m_QueryOffset[m_FrameIndex];
	m_TimeStamps[m_FrameIndex].push_back(ts);
	m_QueryOffset[m_FrameIndex]++;
}

void VulkanProfiler::Print() {
	if (m_TimeStamps[m_FrameIndex].empty())
		return;
	ImGui::Begin("GPUProfiler");
	uint64_t start = m_TimeStamps[m_FrameIndex][0].timeStamp;
	uint32_t count = (uint32_t)m_TimeStamps[m_FrameIndex].size();
	for (uint32_t i = 1; i < count - 1; ++i) {
		uint64_t ticks = m_TimeStamps[m_FrameIndex][i + 1].timeStamp - m_TimeStamps[m_FrameIndex][i].timeStamp;
		ImGui::Text("%s : %f ms", m_TimeStamps[m_FrameIndex][i].name.c_str(), (ticks * m_TimeStampPeriod) / 1000000.0f);
	}
	ImGui::End();
	m_TimeStamps[m_FrameIndex].clear();
}