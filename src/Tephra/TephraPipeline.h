/*
Copyright 2017-01-27 Henrik Johansson texturehandle@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHERLIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

namespace Tephra {
	class Pipeline {
	public:
		Pipeline();
		~Pipeline();
		void LoadPipelineFromFile(const vk::Device& device, const std::string& filename, vk::Viewport vp, vk::RenderPass renderPass);

		void SetDefaultVertexState(const vk::PipelineVertexInputStateCreateInfo& vertexState);
		void SetDefaultMulitSampleState(const vk::PipelineMultisampleStateCreateInfo& msState);

		std::vector<vk::DescriptorSetLayout>& GetDescriptorSetLayouts();
		vk::Pipeline GetPipeline();
		vk::PipelineLayout GetPipelineLayout();

	private:
		std::vector<vk::ShaderModule> m_Shaders;
		int m_ShaderBits;

		vk::Pipeline m_Pipeline;
		std::vector<vk::DescriptorSetLayout> m_DescSetLayouts;
		vk::PipelineLayout m_PipelineLayout;

		vk::PipelineVertexInputStateCreateInfo m_DefaultVertexState;
		bool m_DefaultVertexStateSet;

		vk::PipelineMultisampleStateCreateInfo m_DefaultMultiSampleState;;
		bool m_DefaultMultiSampleStateSet;

	public:
		//mappings to vulkan
#define TO_VK_BOOL(x) (x) ? 1 : 0; 
		static std::unordered_map<std::string, vk::DescriptorType> ToDescriptorType;
		static std::unordered_map<std::string, vk::BlendFactor> ToBlendFactor;
		static std::unordered_map<std::string, vk::BlendOp> ToBlendOp;
		static std::unordered_map<std::string, vk::LogicOp> ToLogicOp;
		static std::unordered_map<std::string, vk::StencilOp> ToStencilOp;
		static std::unordered_map<std::string, vk::CompareOp> ToCompareOp;
		static std::unordered_map<std::string, vk::PrimitiveTopology> ToPrimitiveTopology;
		static std::unordered_map<int, vk::SampleCountFlagBits> ToSampleCount;
		static std::unordered_map<std::string, vk::CullModeFlagBits> ToCullMode;
		static std::unordered_map<std::string, vk::FrontFace> ToFrontFace;
		static std::unordered_map<std::string, vk::PolygonMode> ToPolygonMode;
		static std::unordered_map<std::string, vk::Format> ToFormat;
		static std::unordered_map<int, vk::ShaderStageFlagBits> ToShaderStage;
		static std::unordered_map<std::string, vk::VertexInputRate> ToVertexInputRate;
	};
};