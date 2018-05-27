#pragma once
#include "RenderQueue.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"
#include "DeviceAllocator.h"
#include "RenderProgram.h"
namespace smug {
	class ShadowMapProgram : public RenderProgram {
	public:
		ShadowMapProgram();
		~ShadowMapProgram();
		virtual void Init(VulkanContext& vc, DeviceAllocator& allocator);
		void Update(DeviceAllocator& allocator, RenderQueue& rq);
		void Render(uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources);
		glm::mat4 GetShadowMatrix(int i) { return m_LightViewProjs[i]; };
		FrameBuffer& GetFrameBuffer() { return m_FrameBuffer; }
	private:
		PipelineState m_State;
		FrameBuffer m_FrameBuffer;
		VkBufferHandle m_UniformBuffer;
		vk::DescriptorSet m_DescSet;
		glm::mat4 m_LightViewProjs[4];

		ResourceHandle m_BoxHandle;
		bool m_DebugMode = false;
		CameraData m_DebugCamData;
	};
}