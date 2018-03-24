#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "VkMemoryAllocator.h"
#include "Texture.h"
namespace smug {
	class SkyBox {
	public:
		SkyBox();
		~SkyBox();
		void Init(const vk::Device& device, const vk::PhysicalDevice& physDev, const std::string& filename, const vk::Viewport& vp, const vk::RenderPass& rp, VkMemoryAllocator& allocator);
		void PrepareUniformBuffer(VulkanCommandBuffer cmdBuffer, VkMemoryAllocator& allocator, const glm::mat4& viewProj, const glm::mat4& world);
		void Render(VulkanCommandBuffer cmdBuffer);
	private:
		PipelineState m_Pipeline;
		VkBufferHandle m_VBO;
		VkBufferHandle m_UBO;
		vk::DescriptorSet m_DescSet;
		vk::DescriptorPool m_DescPool;
		VkTexture m_Texture;
	};
}