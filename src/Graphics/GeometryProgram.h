#pragma once
#include "RenderProgram.h"
#include "VkPipeline.h"
namespace smug {
	class GeometryProgram : public RenderProgram {
	public:
		GeometryProgram() {}
		~GeometryProgram() {}
		virtual void Init(VulkanContext& vc, DeviceAllocator& allocator);
		virtual void Update(DeviceAllocator& allocator, const RenderQueue& rq);
		virtual void Render(CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources);
	private:
		PipelineState m_PipelineState;
	};
}