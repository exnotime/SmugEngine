#pragma once
#include "VulkanContext.h"
#include "DeviceAllocator.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"
namespace smug {
	class RenderProgram {
	public:
		RenderProgram() {}
		~RenderProgram() {}
		//init allocates resources
		virtual void Init(VulkanContext& vc, DeviceAllocator& allocator) = 0;
		//updates the resources needed
		virtual void Update(DeviceAllocator& allocator, RenderQueue& rq) = 0;
		//fills the cmdbuffer with render commands
		virtual void Render( uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources) = 0;
	};
}