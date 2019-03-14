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
#include "volk.h"
#include <AssetLoader/Resources.h>

namespace smug {
struct PipelineStateInfo;

class PipelineState {
  public:
	PipelineState();
	~PipelineState();
	void LoadPipelineFromFile(const VkDevice& device, const std::string& filename, VkRenderPass rp);
	void LoadPipelineFromInfo(const VkDevice& device, const PipelineStateInfo& psInfo, VkRenderPass rp);
	void ReloadPipelineFromFile(const VkDevice& device, const std::string& filename, VkRenderPass rp);
	void SetDefaultVertexState(const VkPipelineVertexInputStateCreateInfo& vertexState);
	void SetDefaultMulitSampleState(const VkPipelineMultisampleStateCreateInfo& msState);

	std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts();
	VkPipeline GetPipeline() const;
	VkPipelineLayout GetPipelineLayout() const;

  private:
	std::vector<VkShaderModule> m_Shaders;
	int m_ShaderBits;

	VkPipeline m_Pipeline;
	std::vector<Descriptor> m_Descriptors;
	std::vector<VkDescriptorSetLayout> m_DescSetLayouts;
	VkPipelineLayout m_PipelineLayout;

	VkPipelineVertexInputStateCreateInfo m_DefaultVertexState;
	bool m_DefaultVertexStateSet;

	VkPipelineMultisampleStateCreateInfo m_DefaultMultiSampleState;
	bool m_DefaultMultiSampleStateSet;

  public:
	//mappings to vulkan
#define TO_VK_BOOL(x) (x) ? 1 : 0;
	/*static std::unordered_map<std::string, VkDescriptorType> ToDescriptorType;
	static std::unordered_map<std::string, VkBlendFactor> ToBlendFactor;
	static std::unordered_map<std::string, VkBlendOp> ToBlendOp;
	static std::unordered_map<std::string, VkLogicOp> ToLogicOp;
	static std::unordered_map<std::string, VkStencilOp> ToStencilOp;
	static std::unordered_map<std::string, VkCompareOp> ToCompareOp;
	static std::unordered_map<std::string, VkPrimitiveTopology> ToPrimitiveTopology;
	static std::unordered_map<int, VkSampleCountFlagBits> ToSampleCount;
	static std::unordered_map<std::string, VkCullModeFlagBits> ToCullMode;
	static std::unordered_map<std::string, VkFrontFace> ToFrontFace;
	static std::unordered_map<std::string, VkPolygonMode> ToPolygonMode;
	static std::unordered_map<std::string, VkFormat> ToFormat;
	static std::unordered_map<int, VkShaderStageFlagBits> ToShaderStage;
	static std::unordered_map<std::string, VkVertexInputRate> ToVertexInputRate;*/
};
};