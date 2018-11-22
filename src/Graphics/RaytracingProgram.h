#pragma once
#include "VulkanContext.h"
#include "VkPipeline.h"
namespace smug {
	class RaytracingProgram {
	public:
		RaytracingProgram();
		~RaytracingProgram();
		void Init(vk::Device device, vk::Instance instance);
		void Render(CommandBuffer* cmdBuffer);
	private:
		vk::Pipeline m_Pipeline;
		vk::DescriptorSetLayout m_DescLayout;
		vk::PipelineLayout m_PipelineLayout;
		//vk::AccelerationStructureNV _tlas;
		//vk::AccelerationStructureNV _blas;
	};
}