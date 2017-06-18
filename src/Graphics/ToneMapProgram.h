#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"
class ToneMapProgram {
public:
	ToneMapProgram();
	~ToneMapProgram();
	//swapchain fbo can be bigger or smaller than the fbo we use for rendering
	void Init(vk::Device& device,const glm::vec2& screenSize, FrameBuffer& fbo, vk::DescriptorPool& descPool, vk::RenderPass& rp);
	void Render(VulkanCommandBuffer& cmdBuffer, uint32_t frameIndex);
private:
	Tephra::VkPipeline m_Pipeline;
	vk::DescriptorSet m_DescSet[BUFFER_COUNT];
	vk::Sampler m_Sampler;
};