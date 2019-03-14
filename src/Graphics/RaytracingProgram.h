#pragma once
#if defined(RTX_ON)
#include "VulkanContext.h"
#include "VkPipeline.h"
#include "DeviceAllocator.h"
namespace smug {
	class RaytracingProgram {
	public:
		RaytracingProgram();
		~RaytracingProgram();
		void Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView frameBufferDesc, DeviceAllocator* allocator);
		void Update(CommandBuffer* cmdBuffer, DeviceAllocator* allocator);
		void Render(CommandBuffer* cmdBuffer);
	private:
		VkPipeline m_Pipeline;
		VkDescriptorSetLayout m_DescLayout;
		VkDescriptorSet m_DescSet;
		VkPipelineLayout m_PipelineLayout;
		VkAccelerationStructureNV m_tlas;
		VkAccelerationStructureNV m_blas;
		VkAccelerationStructureInfoNV m_blasInfo;

		VkBufferHandle m_Indices;
		VkBufferHandle m_VertexPositions;
		VkBufferHandle m_VertexNormals;
		VkBufferHandle m_VertexUVs;
		VkBufferHandle m_InstanceBuffer;
		VkBufferHandle m_InstanceOffsetsBuffer;
		VkBufferHandle m_ShaderBindingTable;
		VkBufferHandle m_UniformBuffer;
		VkBufferHandle m_ScratchBuffer;
		uint32_t m_ShaderGroupSize;
		//for faster building
		VkGeometryNV m_blasGeo;

	};
}
#endif