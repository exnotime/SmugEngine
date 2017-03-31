#pragma once
#include "VulkanContext.h"
#include "TephraPipeline.h"
#include "Memory.h"
class Raymarcher {
public: 
	Raymarcher();
	~Raymarcher();

	void Init(const vk::Device& device, const VulkanSwapChain& swapChain, const vk::PhysicalDevice& physDev);
	void UpdateUniforms(VulkanCommandBuffer& cmdBuffer, const glm::mat4& viewProj, const glm::vec3& position);
	void Render(VulkanCommandBuffer& cmdBuffer, uint32_t frameIndex);
private:
	Tephra::Pipeline m_Pipeline;
	vk::Image m_TargetImage;
	vk::DescriptorPool m_DescPool;
	vk::DescriptorSet m_DescSets[BUFFER_COUNT];
	vk::ImageView m_DepthViews[BUFFER_COUNT];
	vk::Sampler m_DepthSampler;
	Memory m_BufferMem;
	Buffer m_UniformBuffer;

	struct PerFrame {
		glm::mat4 invViewProj;
		glm::vec4 CamPos;
	};
};