#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "DeviceAllocator.h"
#include "Texture.h"
namespace smug {
class SkyBox {
  public:
	SkyBox();
	~SkyBox();
	void Init(const vk::Device& device, const vk::PhysicalDevice& physDev, const std::string& filename, const vk::Viewport& vp, const vk::RenderPass& rp, DeviceAllocator& allocator);
	void PrepareUniformBuffer(CommandBuffer* cmdBuffer, DeviceAllocator& allocator, const glm::mat4& viewProj, const glm::mat4& world);
	void Render(CommandBuffer* cmdBuffer);
	void DeInit(DeviceAllocator& allocator);
  private:
	PipelineState m_Pipeline;
	VkBufferHandle m_VBO;
	VkBufferHandle m_UBO;
	vk::DescriptorSet m_DescSet;
	vk::DescriptorPool m_DescPool;
	VkTexture m_Texture;
};
}