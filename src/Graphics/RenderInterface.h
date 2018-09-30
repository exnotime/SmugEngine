#pragma once
#include <vector>
namespace smug {
	//c interface for executing render commmands
	namespace RenderInterface {

		//void CreateDescriptorSetLayout(DescriptorSetLayoutInfo & info);
		void CreatePipelineState();
		void CreateFrameBuffer();

		void BindFramebuffer();
		void BindPipelineState();
		void BindViewportScissor();
		void BindDescriptorSets();
		
		void Draw();
		void Disbatch();
	}
}