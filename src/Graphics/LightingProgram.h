#pragma once
#include "volk.h"
#include "DeviceAllocator.h"
#include <glm/glm.hpp>
namespace smug {
	class ResourceHandler;
	class FrameBufferManager;
	typedef uint64_t ResourceHandle;

	class LightingProgram {
	public:
		LightingProgram();
		~LightingProgram();
		void Init(VkDevice device, VkDescriptorPool descPool, ResourceHandler* resources, FrameBufferManager* framebuffers, DeviceAllocator* allocator);
		void DeInit();
		void Update(DeviceAllocator* allocator, const glm::vec3& lightDir, const glm::mat4& invviewProj, const glm::vec3& camPos, const glm::vec2& screenSize);
		void Render(VkCommandBuffer cmdBuffer, ResourceHandler* resources, VkDescriptorSet iblSet, const glm::vec2& screenSize);
	private:
		ResourceHandle m_PipelineState;
		VkBufferHandle m_UniformBuffer;
		VkDescriptorSet m_DescSet;
		VkSampler m_Sampler;
	};
}