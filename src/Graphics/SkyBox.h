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
	void Init(const VkDevice& device, const VkPhysicalDevice& physDev, const eastl::string& filename, const VkViewport& vp, const VkRenderPass& rp, DeviceAllocator& allocator);
	void PrepareUniformBuffer(DeviceAllocator& allocator, const glm::mat4& viewProj, const glm::mat4& world);
	void Render(CommandBuffer* cmdBuffer);
	void DeInit(DeviceAllocator& allocator);
  private:
	PipelineState m_Pipeline;
	VkBufferHandle m_VBO;
	VkBufferHandle m_UBO;
	VkDescriptorSet m_DescSet;
	VkDescriptorPool m_DescPool;
	VkTexture m_Texture;
};
}