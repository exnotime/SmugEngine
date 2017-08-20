#include "RenderQueue.h"

RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::Init(VkMemory& memory) {
	//allocate gpu memory for shader inputs
	vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eStorageBuffer;
	m_VkBuffer = memory.AllocateBuffer(4 * MEGA_BYTE, flags, nullptr);

	m_Cameras.reserve(10);
	m_Models.reserve(10000);
	m_Inputs.reserve(10000);
}

void RenderQueue::Clear() {
	m_Cameras.clear();
	m_Models.clear();
	m_Inputs.clear();
}

void RenderQueue::AddCamera(const CameraData & cd) {
	m_Cameras.push_back(cd);
}

void RenderQueue::AddModel(ResourceHandle model, const glm::mat4& transform, const glm::vec4& tint) {
	m_Models.push_back(model);
	m_Inputs.push_back({transform, tint});
}

void RenderQueue::ScheduleTransfer(VkMemory& memory) {
	if(m_Inputs.size() > 0)
	memory.UpdateBuffer(m_VkBuffer, m_Inputs.size() * sizeof(ShaderInput), m_Inputs.data());
}
