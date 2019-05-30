#pragma once
#include "VulkanContext.h"
#include "DeviceAllocator.h"
#include "ResourceHandler.h"

#if defined(RTX_ON)
namespace smug {
	struct CameraData;
	class CommandBuffer;
	struct CameraData;
	class RenderQueue;
	class RaytracingProgram {
	public:
		RaytracingProgram() {}
		~RaytracingProgram(){}
		void Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView outputTarget, VkImageView depthStencil, VkImageView normals, DeviceAllocator& allocator);
		void AddRenderQueueToTLAS(RenderQueue* queue, bool staticRenderQueue, DeviceAllocator& allocator, ResourceHandler& resources);
		void BuildTLAS(CommandBuffer* cmdBuffer);
		void Update(DeviceAllocator* allocator, const CameraData* cd, const glm::vec3& lightDir);
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

		VkSampler m_Sampler;

		uint32_t m_StaticInstanceCount = 0;
	};
}
#endif