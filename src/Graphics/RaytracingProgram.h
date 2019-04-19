#pragma once
#include "VulkanContext.h"
#include "DeviceAllocator.h"

#if defined(RTX_ON)
namespace smug {
	struct CameraData;
	class CommandBuffer;
	class DeviceAllocator;
	struct CameraData;
	class RenderQueue;
	class RaytracingProgram {
	public:
		RaytracingProgram() {}
		~RaytracingProgram(){}
		void Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView frameBufferDesc, DeviceAllocator& allocator);
		void AddRenderQueueToTLAS(RenderQueue* queue, bool staticRenderQueue, DeviceAllocator& allocator);
		void BuildTLAS(CommandBuffer* cmdBuffer);
		void Update(CommandBuffer* cmdBuffer, DeviceAllocator* allocator, const CameraData* cd);
		void Render(CommandBuffer* cmdBuffer);
		void DeInit(DeviceAllocator* allocator);
	private:
		uint32_t m_ShaderGroupSize = 16;

		VkAccelerationStructureNV m_tlas;
		VkAccelerationStructureNV m_blas;
		VkPipeline m_Pipeline;
		VkDescriptorSetLayout m_DescLayout;
		VkDescriptorSet m_DescSet;
		VkPipelineLayout m_PipelineLayout;

		VkAccelerationStructureInfoNV m_blasInfo;
		VkAccelerationStructureInfoNV m_tlasInfo;

		VkBufferHandle m_Indices;
		VkBufferHandle m_VertexPositions;
		VkBufferHandle m_InstanceBuffer;
		VkBufferHandle m_ShaderBindingTable;
		VkBufferHandle m_UniformBuffer;
		VkBufferHandle m_ScratchBuffer;
		VkBufferHandle m_BLASBuffer;
		VkBufferHandle m_TLASBuffer;
		VkGeometryNV m_blasGeo;

		uint32_t m_StaticInstanceCount = 0;
	};
}
#endif