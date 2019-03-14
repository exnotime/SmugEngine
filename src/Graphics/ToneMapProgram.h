#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"
#include "DeviceAllocator.h"
namespace smug {
struct ToneMapUniformData {
	float bright;
	float exposure;
};

class ToneMapProgram {
  public:
	ToneMapProgram();
	~ToneMapProgram();
	//swapchain fbo can be bigger or smaller than the fbo we use for rendering
	void Init(VkDevice& device, const glm::vec2& screenSize, FrameBufferManager& fbo, VkDescriptorPool& descPool, VkRenderPass& rp, DeviceAllocator& allocator);
	void DeInit(DeviceAllocator& allocator);
	void Update(DeviceAllocator& allocator);
	void Render(CommandBuffer& cmdBuffer, VkViewport viewport, uint32_t frameIndex);
  private:
	PipelineState m_Pipeline;
	VkDescriptorSet m_DescSet[BUFFER_COUNT];
	VkSampler m_Sampler;
	VkBufferHandle m_Buffer;
};
}