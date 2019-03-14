#pragma once
#include "RenderQueue.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"
#include "DeviceAllocator.h"
#include "RenderProgram.h"
#include "VulkanProfiler.h"
namespace smug {
class ShadowMapProgram {
  public:
	ShadowMapProgram();
	~ShadowMapProgram();
	virtual void Init(VulkanContext& vc, DeviceAllocator& allocator);
	void DeInit(DeviceAllocator& allocator);
	void Update(DeviceAllocator& allocator, RenderQueue& rq);
	void Render(uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources, VulkanProfiler& profiler);
	glm::mat4 GetShadowMatrix(int i) {
		return m_LightViewProjs[i];
	};
	glm::vec4 GetShadowSplits() {
		return m_ShadowSplits;
	}
	FrameBufferManager& GetFrameBuffer() {
		return m_FrameBuffer;
	}

	void Recompile() { m_State.ReloadPipelineFromFile(*m_Device, "shader/shadowmap.json", m_FrameBuffer.GetRenderPass()); }
  private:
	PipelineState m_State;
	FrameBufferManager m_FrameBuffer;
	VkBufferHandle m_UniformBuffer;
	VkDescriptorSet m_DescSet;
	glm::mat4 m_LightViewProjs[4];

	ResourceHandle m_BoxHandle;
	bool m_DebugMode = false;
	CameraData m_DebugCamData;
	VkDevice* m_Device;
	glm::vec4 m_ShadowSplits;

	glm::vec4 m_FrustumMax[4], m_FrustumMin[4];
};
}