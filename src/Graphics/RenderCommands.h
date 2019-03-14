#pragma once
#include "RenderPipeline.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"

namespace smug {
	namespace RenderCommands {
		void RenderCommand(const RenderCmd& cmd, VkCommandBuffer cmdBuffer, RenderQueue& rq, ResourceHandler& resources, const VkDescriptorSet& perFrameSet, const VkDescriptorSet& iblSet);
		void DisptachCommand(const DispatchCmd& cmd, VkCommandBuffer cmdBuffer, ResourceHandler& resources);
		void CopyCommands(const CopyCmd& cmd, VkCommandBuffer cmdBuffer, ResourceHandler& resources);
		void FenceCommand(const FenceCmd& cmd, VkDevice device);
	}
}