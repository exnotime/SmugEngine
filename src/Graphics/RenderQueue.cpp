#include "RenderQueue.h"
using namespace smug;
RenderQueue::RenderQueue() {
}

RenderQueue::~RenderQueue() {
}

void RenderQueue::Init(DeviceAllocator& memory) {
	//allocate gpu memory for shader inputs
	m_Buffer = memory.AllocateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 1024 * 1024, nullptr);

	m_Cameras.reserve(10);
	m_Inputs.reserve(1024);
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
	auto& m = m_Models.find(model);
	if (m == m_Models.end()) {
		m_Models[model] = ModelInstance();
		m_Models[model].Count = 0;
	}
	m_Models[model].Count++;
	ShaderInput i = { transform, tint };
	m_Models[model].Inputs.push_back(i);
}

void RenderQueue::ScheduleTransfer(DeviceAllocator& memory) {
	for (auto& mi : m_Models) {
		mi.second.Offset = (uint32_t)m_Inputs.size();
		m_Inputs += mi.second.Inputs;
	}

	if (m_Inputs.size() > 0) {
		memory.UpdateBuffer(m_Buffer, m_Inputs.size() * sizeof(ShaderInput), m_Inputs.data());
		m_Inputs.clear();
	}

}
