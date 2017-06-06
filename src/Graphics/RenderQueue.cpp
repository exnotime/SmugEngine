#include "RenderQueue.h"

RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::Init(VkMemory& memory) {
	//allocate gpu memory for shader inputs
	vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eStorageBuffer;
	m_VkBuffer = memory.AllocateBuffer(4 * MEGA_BYTE, flags, nullptr);
}

void RenderQueue::Clear() {
	m_Cameras.clear();
	m_Models.clear();
	m_Inputs.clear();
	m_Spheres.clear();
}

void RenderQueue::AddCamera(const CameraData & cd) {
	m_Cameras.push_back(cd);
}

void RenderQueue::AddModel(ResourceHandle handle, const ShaderInput& input) {
	m_Models.push_back(handle);
	m_Inputs.push_back(input);
}

void RenderQueue::ScheduleTransfer(VkMemory& memory) {
	memory.UpdateBuffer(m_VkBuffer, m_Inputs.size() * sizeof(ShaderInput), m_Inputs.data());
}
