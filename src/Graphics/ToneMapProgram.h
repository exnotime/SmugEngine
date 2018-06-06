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
	void Init(vk::Device& device, const glm::vec2& screenSize, FrameBuffer& fbo, vk::DescriptorPool& descPool, vk::RenderPass& rp, DeviceAllocator& allocator);
	void Update(DeviceAllocator& allocator);
	void Render(CommandBuffer& cmdBuffer, vk::Viewport viewport, uint32_t frameIndex);
  private:
	PipelineState m_Pipeline;
	vk::DescriptorSet m_DescSet[BUFFER_COUNT];
	vk::Sampler m_Sampler;
	VkBufferHandle m_Buffer;
};
}