#pragma once
#include "VulkanContext.h"
#include "TephraPipeline.h"
#include "Memory.h"
#include "Texture.h"
class SkyBox {
public:
	SkyBox();
	~SkyBox();
	void Init(const vk::Device& device, const vk::PhysicalDevice& physDev, const std::string& filename,const vk::Viewport& vp, const vk::RenderPass& rp,const vk::PipelineMultisampleStateCreateInfo& mss);
	void PrepareUniformBuffer(VulkanCommandBuffer cmdBuffer);
	void Render(VulkanCommandBuffer cmdBuffer);
private:
	Tephra::Pipeline m_Pipeline;
	Memory m_Memory;
	Buffer m_VBO;
	Buffer m_UBO;
	vk::DescriptorSet m_DescSet;
	vk::DescriptorPool m_DescPool;
	Texture m_Texture;
};