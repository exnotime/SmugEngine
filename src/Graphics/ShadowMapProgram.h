#pragma once
#include "RenderQueue.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"
namespace smug {
	class ShadowMapProgram {
	public:
		ShadowMapProgram();
		~ShadowMapProgram();
		void Init(vk::Device device, vk::PhysicalDevice gpu, vk::Format format);
		void Render(const RenderQueue& rq,const DirLight& light);
	private:
		vk::RenderPass m_RenderPass;
		PipelineState m_State;
		FrameBuffer m_FrameBuffer;
	};
}