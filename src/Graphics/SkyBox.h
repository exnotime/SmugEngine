#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "VkMemory.h"
#include "Texture.h"
class SkyBox {
public:
	SkyBox();
	~SkyBox();
	void Init(const vk::Device& device, const vk::PhysicalDevice& physDev, const std::string& filename,const vk::Viewport& vp, const vk::RenderPass& rp,const vk::PipelineMultisampleStateCreateInfo& mss);
	void PrepareUniformBuffer(VulkanCommandBuffer cmdBuffer, glm::mat4 viewProj, glm::mat4 world);
	void Render(VulkanCommandBuffer cmdBuffer);
private:
	Tephra::VkPipeline m_Pipeline;
	VkMemory m_Memory;
	VkAlloc m_VBO;
	VkAlloc m_UBO;
	vk::DescriptorSet m_DescSet;
	vk::DescriptorPool m_DescPool;
	VkTexture m_Texture;
};