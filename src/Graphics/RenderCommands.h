#pragma once
#include "RenderPipeline.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"

namespace smug {
	namespace RenderCommands {
		void RenderCommand(const RenderCmd& cmd, vk::CommandBuffer cmdBuffer, RenderQueue& rq, ResourceHandler& resources, const vk::DescriptorSet& perFrameSet, const vk::DescriptorSet& iblSet);
		void DisptachCommand(const DispatchCmd& cmd, vk::CommandBuffer cmdBuffer);
		void CopyCommands();
		void FenceCommand();
	}
}