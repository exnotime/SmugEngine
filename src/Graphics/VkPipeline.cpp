#include "VkPipeline.h"
#include <fstream>
#include <sstream>
#include <json.hpp>
#include "VkShader.h"
#include "PipelineCommon.h"
#include <AssetLoader/AssetLoader.h>
using nlohmann::json;
using namespace smug;

enum SHADER_TYPES {
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	EVALUATION_SHADER,
	CONTROL_SHADER,
	COMPUTE_SHADER
};

PipelineState::PipelineState() {
	m_DefaultVertexStateSet = false;
	m_DefaultMultiSampleStateSet = false;
}

PipelineState::~PipelineState() {
	m_Shaders.clear();
}

vk::PipelineColorBlendAttachmentState ReadColorBlendAttachmentState(const json& blendJson) {
	vk::PipelineColorBlendAttachmentState blendAttachStateInfo = GetDefaultColorBlendAttachmentState();
	if (blendJson.find("BlendEnable") != blendJson.end()) {
		blendAttachStateInfo.blendEnable = TO_VK_BOOL(blendJson["BlendEnable"]);
	}
	if (blendJson.find("SrcColorBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.srcColorBlendFactor = ToBlendFactor[blendJson["SrcColorBlendFactor"]];
	}
	if (blendJson.find("DstColorBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.dstColorBlendFactor = ToBlendFactor[blendJson["DstColorBlendFactor"]];
	}
	if (blendJson.find("ColorBlendOp") != blendJson.end()) {
		blendAttachStateInfo.colorBlendOp = ToBlendOp[blendJson["ColorBlendOp"]];
	}
	if (blendJson.find("SrcAlphaBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.srcAlphaBlendFactor = ToBlendFactor[blendJson["SrcAlphaBlendFactor"]];
	}
	if (blendJson.find("DstAlphaBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.dstAlphaBlendFactor = ToBlendFactor[blendJson["DstAlphaBlendFactor"]];
	}
	if (blendJson.find("AlphaBlendOp") != blendJson.end()) {
		blendAttachStateInfo.alphaBlendOp = ToBlendOp[blendJson["AlphaBlendOp"]];
	}
	//add color write mask
	return blendAttachStateInfo;
}

vk::PipelineColorBlendStateCreateInfo ReadColorBlendState(const json& blendState) {
	vk::PipelineColorBlendStateCreateInfo blendStateInfo = GetDefaultColorBlendState();
	if (blendState.find("LogicOp") != blendState.end()) {
		blendStateInfo.logicOp = ToLogicOp[blendState["LogicOp"]];
	}
	if (blendState.find("logicOpEnable") != blendState.end()) {
		blendStateInfo.logicOpEnable = TO_VK_BOOL(blendState["logicOpEnable"]);
	}
	if (blendState.find("BlendConstants") != blendState.end()) {
		blendStateInfo.blendConstants[0] = blendState["BlendConstants"][0];
		blendStateInfo.blendConstants[1] = blendState["BlendConstants"][1];
		blendStateInfo.blendConstants[2] = blendState["BlendConstants"][2];
		blendStateInfo.blendConstants[3] = blendState["BlendConstants"][3];
	}
	return blendStateInfo;
}

vk::PipelineDepthStencilStateCreateInfo ReadDepthStencilstate(const json& depthState) {
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo = GetDefaultDepthStencilstate();
	if (depthState.find("Back") != depthState.end()) {
		depthStencilCreateInfo.back = ToStencilOp[depthState["Back"]];
	}
	if (depthState.find("Front") != depthState.end()) {
		depthStencilCreateInfo.front = ToStencilOp[depthState["Front"]];
	}
	if (depthState.find("DepthBoundsTestEnable") != depthState.end()) {
		depthStencilCreateInfo.depthBoundsTestEnable = TO_VK_BOOL(depthState["DepthBoundsTestEnable"]);
	}
	if (depthState.find("DepthCompareOp") != depthState.end()) {
		depthStencilCreateInfo.depthCompareOp = ToCompareOp[depthState["DepthCompareOp"]];
	}
	if (depthState.find("DepthTestEnable") != depthState.end()) {
		depthStencilCreateInfo.depthTestEnable = TO_VK_BOOL(depthState["DepthTestEnable"]);
	}
	if (depthState.find("DepthWriteEnable") != depthState.end()) {
		depthStencilCreateInfo.depthWriteEnable = TO_VK_BOOL(depthState["DepthWriteEnable"]);
	}
	if (depthState.find("MaxDepthBounds") != depthState.end()) {
		depthStencilCreateInfo.maxDepthBounds = depthState["MaxDepthBounds"];
	}
	if (depthState.find("MinDepthBounds") != depthState.end()) {
		depthStencilCreateInfo.minDepthBounds = depthState["MinDepthBounds"];
	}
	return depthStencilCreateInfo;
}

vk::PipelineMultisampleStateCreateInfo GetDefaultMultiSampleState() {
	vk::PipelineMultisampleStateCreateInfo msState;
	msState.rasterizationSamples = vk::SampleCountFlagBits::e1;
	msState.alphaToCoverageEnable = false;
	msState.alphaToOneEnable = false;
	msState.minSampleShading = 0.0f;
	msState.sampleShadingEnable = false;
	msState.pSampleMask = nullptr;
	return msState;
}

vk::PipelineMultisampleStateCreateInfo ReadMultiSampleState(const json& msStateJson) {
	vk::PipelineMultisampleStateCreateInfo msState = GetDefaultMultiSampleState();
	if (msStateJson.find("RasterizationSamples") != msStateJson.end()) {
		msState.rasterizationSamples = ToSampleCount[msStateJson["RasterizationSamples"]];
	}
	if (msStateJson.find("SampleShadingEnable") != msStateJson.end()) {
		msState.sampleShadingEnable = TO_VK_BOOL(msStateJson["SampleShadingEnable"]);
	}
	if (msStateJson.find("MinSampleShading") != msStateJson.end()) {
		msState.minSampleShading = msStateJson["MinSampleShading"];
	}
	if (msStateJson.find("AlphaToCoverageEnable") != msStateJson.end()) {
		msState.alphaToCoverageEnable = TO_VK_BOOL(msStateJson["AlphaToCoverageEnable"]);
	}
	if (msStateJson.find("AlphaToOneEnable") != msStateJson.end()) {
		msState.alphaToOneEnable = TO_VK_BOOL(msStateJson["AlphaToOneEnable"]);
	}
	return msState;
}

vk::PipelineRasterizationStateCreateInfo ReadRasterState(const json& rasterStateJson) {
	vk::PipelineRasterizationStateCreateInfo rState = GetDefaultRasterState();
	if (rasterStateJson.find("CullMode") != rasterStateJson.end()) {
		rState.cullMode = ToCullMode[rasterStateJson["CullMode"]];
	}
	if (rasterStateJson.find("FrontFace") != rasterStateJson.end()) {
		rState.frontFace = ToFrontFace[rasterStateJson["FrontFace"]];
	}
	if (rasterStateJson.find("PolygonMode") != rasterStateJson.end()) {
		rState.polygonMode = ToPolygonMode[rasterStateJson["PolygonMode"]];
	}
	if (rasterStateJson.find("DepthClampEnable") != rasterStateJson.end()) {
		rState.depthClampEnable = TO_VK_BOOL(rasterStateJson["DepthClampEnable"]);
	}
	if (rasterStateJson.find("RasterizerDiscardEnable") != rasterStateJson.end()) {
		rState.rasterizerDiscardEnable = TO_VK_BOOL(rasterStateJson["RasterizerDiscardEnable"]);
	}
	if (rasterStateJson.find("DepthBiasEnable") != rasterStateJson.end()) {
		rState.depthBiasEnable = TO_VK_BOOL(rasterStateJson["DepthBiasEnable"]);
	}
	if (rasterStateJson.find("DepthBiasConstantFactor") != rasterStateJson.end()) {
		rState.depthBiasConstantFactor = rasterStateJson["DepthBiasConstantFactor"];
	}
	if (rasterStateJson.find("DepthBiasClamp") != rasterStateJson.end()) {
		rState.depthBiasClamp = rasterStateJson["DepthBiasClamp"];
	}
	if (rasterStateJson.find("DepthBiasSlopeFactor") != rasterStateJson.end()) {
		rState.depthBiasSlopeFactor = rasterStateJson["DepthBiasSlopeFactor"];
	}
	if (rasterStateJson.find("LineWidth") != rasterStateJson.end()) {
		rState.lineWidth = rasterStateJson["LineWidth"];
	}
	return rState;
}

void PipelineState::LoadPipelineFromFile(const vk::Device& device, const std::string& filename, vk::RenderPass renderPass) {
	std::ifstream fin(filename);
	if (!fin.is_open()) {
		printf("Error opening pipeline file %s\n", filename.c_str());
		fin.close();
		return;
	}
	json root;
	try {
		fin >> root;
	} catch(std::exception e) {
		printf("json: %s\n", e.what());
		return;
	}
	fin.close();
	//Find shaders
	m_ShaderBits = 0;
	if (root.find("Shaders") != root.end()) {
		json shaders = root["Shaders"];

		std::string shader_types[] = { "Vertex", "Fragment", "Geometry", "EVALUATION", "Control", "Compute" };
		for (int i = 0; i < COMPUTE_SHADER + 1; ++i) {
			if (shaders.find(shader_types[i]) != shaders.end()) {
				json shader = shaders[shader_types[i]];
				std::string entry;
				SHADER_LANGUAGE lang = GLSL;

				if (shader.find("EntryPoint") != shader.end()) {
					entry = shader["EntryPoint"];
				} else {
					entry = "main";
				}
				if (shader.find("Language") != shader.end()) {
					if (shader["Language"] == "GLSL") {
						lang = GLSL;
					} else if (shader["Language"] == "HLSL") {
						lang = HLSL;
					}
				}

				m_Shaders.push_back(LoadShader(device, shader["Source"], SHADER_KIND(i), entry, lang));
				m_ShaderBits |= 1 << i;
			} else {
				m_Shaders.push_back(vk::ShaderModule());
			}
		}
	} else {
		return;
	}
	//descriptor set
	if (root.find("DescriptorSetLayouts") != root.end()) {
		json descSetLayouts = root["DescriptorSetLayouts"];
		for (auto& layout : descSetLayouts) {
			std::vector<vk::DescriptorSetLayoutBinding> bindings;

			for (auto& bind : layout) {
				vk::DescriptorSetLayoutBinding binding;
				binding.binding = bind["Binding"];
				binding.descriptorCount = bind["Count"];
				binding.descriptorType = ToDescriptorType[bind["Type"]];
				binding.stageFlags = vk::ShaderStageFlagBits::eAll; //keep the stage set to all for now
				bindings.push_back(binding);
			}
			vk::DescriptorSetLayoutCreateInfo descSetCreateInfo;
			descSetCreateInfo.bindingCount = (uint32_t)bindings.size();
			descSetCreateInfo.pBindings = bindings.data();
			vk::DescriptorSetLayout layout;
			layout = device.createDescriptorSetLayout(descSetCreateInfo);
			m_DescSetLayouts.push_back(layout);
		}
	}
	//push constants
	std::vector<vk::PushConstantRange> pushConstRanges;
	if (root.find("PushConstantRanges") != root.end()) {
		json pushConstants = root["PushConstantRanges"];
		for (auto& pc : pushConstants) {
			vk::PushConstantRange pushConst;
			pushConst.stageFlags = vk::ShaderStageFlagBits::eAll; //keep the stage set to all for now
			pushConst.size = pc["Size"];
			pushConst.offset = pc["Offset"];
			pushConstRanges.push_back(pushConst);
		}
	}
	//pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstRanges.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstRanges.size();
	pipelineLayoutCreateInfo.pSetLayouts = m_DescSetLayouts.data();
	pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)m_DescSetLayouts.size();
	m_PipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);
	//if we use a compute shader we dont need anything else
	if (m_ShaderBits & (1 << COMPUTE_SHADER)) {
		vk::ComputePipelineCreateInfo pipelineInfo;
		pipelineInfo.layout = m_PipelineLayout;
		vk::PipelineShaderStageCreateInfo computeStage;
		computeStage.module = m_Shaders[0];
		computeStage.stage = vk::ShaderStageFlagBits::eCompute;
		computeStage.pName = "main";
		pipelineInfo.stage = computeStage;
		m_Pipeline = device.createComputePipeline(nullptr, pipelineInfo);
		return;
	}

	//color blend state
	vk::PipelineColorBlendStateCreateInfo blendStateInfo;
	std::vector<vk::PipelineColorBlendAttachmentState> ColorblendAttachmentStates;
	if (root.find("ColorBlendState") != root.end()) {
		json ColorBlendStateJson = root["ColorBlendState"];
		blendStateInfo = ReadColorBlendState(ColorBlendStateJson);
		//blend attachments
		if (ColorBlendStateJson.find("ColorBlendAttachmentStates") != ColorBlendStateJson.end()) {
			json blendAttachmentStatesJson = ColorBlendStateJson["ColorBlendAttachmentStates"];
			for (auto& state : blendAttachmentStatesJson) {
				ColorblendAttachmentStates.push_back(ReadColorBlendAttachmentState(state));
			}
		} else {
			ColorblendAttachmentStates.push_back(GetDefaultColorBlendAttachmentState());
		}
	} else {
		blendStateInfo = GetDefaultColorBlendState();
	}
	blendStateInfo.attachmentCount = (uint32_t)ColorblendAttachmentStates.size();
	blendStateInfo.pAttachments = ColorblendAttachmentStates.data();
	//depth stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencilState;
	if (root.find("DepthStencilState") != root.end()) {
		json depthStencilJson = root["DepthStencilState"];
		depthStencilState = ReadDepthStencilstate(depthStencilJson);
	} else {
		depthStencilState = GetDefaultDepthStencilstate();
	}
	//input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	if (root.find("InputAssemblyState") != root.end()) {
		json inputAssemJson = root["InputAssemblyState"];
		if(inputAssemJson.find("PrimitiveRestartEnable") != inputAssemJson.end()) {
			inputAssemblyInfo.primitiveRestartEnable = TO_VK_BOOL(inputAssemJson["PrimitiveRestartEnable"]);
		} else {
			inputAssemblyInfo.primitiveRestartEnable = false;
		}
		if (inputAssemJson.find("Topology") != inputAssemJson.end()) {
			inputAssemblyInfo.topology = ToPrimitiveTopology[ inputAssemJson["Topology"]];
		} else {
			inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		}
	} else {
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyInfo.primitiveRestartEnable = false;
	}
	// ms state
	vk::PipelineMultisampleStateCreateInfo* multisampleStateInfo = nullptr;
	if (root.find("MultiSampleState") != root.end()) {
		json msStateJson = root["MultiSampleState"];
		multisampleStateInfo = new vk::PipelineMultisampleStateCreateInfo();
		*multisampleStateInfo = ReadMultiSampleState(msStateJson);
	} else if (m_DefaultMultiSampleStateSet) {
		// do nothing
	} else {
		multisampleStateInfo = new vk::PipelineMultisampleStateCreateInfo();
		*multisampleStateInfo = GetDefaultMultiSampleState();
	}
	//raster state
	vk::PipelineRasterizationStateCreateInfo rasterInfo;
	if (root.find("RasterizationState") != root.end()) {
		json rsStateJson = root["RasterizationState"];
		rasterInfo = ReadRasterState(rsStateJson);
	} else {
		rasterInfo = GetDefaultRasterState();
	}
	//tesselation state
	vk::PipelineTessellationStateCreateInfo* tesselationstate = nullptr;
	if (root.find("TesselationState") != root.end()) {
		json tStateJson = root["TesselationState"];
		tesselationstate = new vk::PipelineTessellationStateCreateInfo();
		if (tStateJson.find("PatchControlPoints") != tStateJson.end()) {
			tesselationstate->patchControlPoints = tStateJson["PatchControlPoints"];
		} else {
			tesselationstate->patchControlPoints = 1;
		}
	}
	//shader stages
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	for (int i = 0; i < 6; i++) {
		vk::PipelineShaderStageCreateInfo stage;
		int shaderIndex = 0;
		if (m_ShaderBits & (1 << i)) {
			stage.module = m_Shaders[i];
			stage.stage = ToShaderStage[i];
			stage.pName = "main";
			stage.pSpecializationInfo = nullptr;
			shaderStages.push_back(stage);
			shaderIndex++;
		}
	}
	//vertex state
	//Vertex state must either be set in json or provided with a default before calling this function
	vk::PipelineVertexInputStateCreateInfo* vertexState = nullptr;
	std::vector<vk::VertexInputBindingDescription> vertexBindings;
	std::vector<vk::VertexInputAttributeDescription> vertexInputDescs;
	if (root.find("VertexInputState") != root.end()) {
		json vertexStateJson = root["VertexInputState"];
		//bindings
		if (vertexStateJson.find("InputBindings") != vertexStateJson.end()) {
			json bindings = vertexStateJson["InputBindings"];
			for (auto& bind : bindings) {
				vk::VertexInputBindingDescription bindDesc;
				bindDesc.binding = bind["Binding"];
				bindDesc.inputRate = ToVertexInputRate[bind["InputRate"]];
				bindDesc.stride = bind["Stride"];
				vertexBindings.push_back(bindDesc);
			}
		}
		//input attributes
		if (vertexStateJson.find("InputAttributes") != vertexStateJson.end()) {
			json attributes = vertexStateJson["InputAttributes"];
			for (auto& attr : attributes) {
				vk::VertexInputAttributeDescription attrDesc;
				attrDesc.binding = attr["Binding"];
				attrDesc.format = ToFormat[attr["Format"]];
				attrDesc.location = attr["Location"];
				attrDesc.offset = attr["Offset"];
				vertexInputDescs.push_back(attrDesc);
			}
		}
		vertexState = new vk::PipelineVertexInputStateCreateInfo();
		vertexState->pVertexBindingDescriptions = vertexBindings.data();
		vertexState->vertexBindingDescriptionCount = (uint32_t)vertexBindings.size();
		vertexState->pVertexAttributeDescriptions = vertexInputDescs.data();
		vertexState->vertexAttributeDescriptionCount = (uint32_t)vertexInputDescs.size();
	}

	//viewport state
	vk::PipelineViewportStateCreateInfo viewportState;
	if (root.find("ViewportCount") != root.end()) {
		viewportState.viewportCount = root["ViewportCount"];
	} else {
		viewportState.viewportCount = 1;
	}
	if (root.find("ScissorCount") != root.end()) {
		viewportState.scissorCount = root["ScissorCount"];
	} else {
		viewportState.scissorCount = 1;
	}
	viewportState.pViewports = nullptr;
	viewportState.pScissors = nullptr;

	std::vector<vk::DynamicState> dynamicStates;
	dynamicStates.push_back(vk::DynamicState::eViewport);
	dynamicStates.push_back(vk::DynamicState::eScissor);
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	//put everything together
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.pColorBlendState = &blendStateInfo;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pTessellationState = (tesselationstate == nullptr) ? tesselationstate : nullptr;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pMultisampleState = (multisampleStateInfo == nullptr) ? &m_DefaultMultiSampleState : multisampleStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterInfo;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = (vertexState == nullptr) ? &m_DefaultVertexState : vertexState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();

	m_Pipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);

	//clean up
	if (vertexState) delete vertexState;
	if (multisampleStateInfo) delete multisampleStateInfo;
	if (tesselationstate) delete tesselationstate;
}

vk::ShaderStageFlagBits ShaderKindToVkStage(SHADER_KIND k) {
	switch (k)
	{
	case smug::VERTEX:
		return vk::ShaderStageFlagBits::eVertex;
		break;
	case smug::FRAGMENT:
		return vk::ShaderStageFlagBits::eFragment;
		break;
	case smug::GEOMETRY:
		return vk::ShaderStageFlagBits::eGeometry;
		break;
	case smug::EVALUATION:
		return vk::ShaderStageFlagBits::eTessellationEvaluation;
		break;
	case smug::CONTROL:
		return vk::ShaderStageFlagBits::eTessellationControl;
		break;
	case smug::COMPUTE:
		return vk::ShaderStageFlagBits::eCompute;
		break;
	}
	return vk::ShaderStageFlagBits::eCompute;

}

void PipelineState::LoadPipelineFromInfo(const vk::Device& device, const PipelineStateInfo& psInfo) {
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;

	vk::PipelineLayoutCreateInfo layoutCreateInfo;
	std::vector<vk::DescriptorSetLayout> setLayouts;
	std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> bindings;

	for (uint32_t i = 0; i < psInfo.DescriptorCount; ++i) {
		Descriptor& desc = psInfo.Descriptors[i];
		vk::DescriptorSetLayoutBinding bind;
		bind.binding = desc.Bindpoint;
		bind.descriptorCount = desc.Count;
		bind.descriptorType = (vk::DescriptorType)desc.Type;
		bind.stageFlags = vk::ShaderStageFlagBits::eAll;
		bool found = false;
		for (auto& desc : bindings[desc.Set]) {
			if (desc == bind) {
				found = true;
				break;
			}
		}
		if(!found)
			bindings[desc.Set].push_back(bind);
	}
	for (uint32_t i = 0; i < bindings.size(); ++i) {
		vk::DescriptorSetLayoutCreateInfo info;
		info.bindingCount = bindings[i].size();
		info.pBindings = bindings[i].data();
		setLayouts.push_back(device.createDescriptorSetLayout(info));
	}
	layoutCreateInfo.pSetLayouts = setLayouts.data();
	layoutCreateInfo.setLayoutCount = setLayouts.size();
	layoutCreateInfo.pushConstantRangeCount = 1;
	vk::PushConstantRange pc;
	pc.offset = psInfo.PushConstants.Offset;
	pc.size = psInfo.PushConstants.Size;
	pc.stageFlags = vk::ShaderStageFlagBits::eAll;
	layoutCreateInfo.pPushConstantRanges = &pc;
	m_PipelineLayout = device.createPipelineLayout(layoutCreateInfo);

	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	for (uint32_t i = 0; i < psInfo.Shader.ShaderCount; ++i) {
		vk::ShaderModuleCreateInfo moduleInfo;
		moduleInfo.codeSize = psInfo.Shader.Shaders[i].ByteCodeSize;
		moduleInfo.pCode = (uint32_t*)psInfo.Shader.Shaders[i].ByteCode;
		vk::PipelineShaderStageCreateInfo shaderInfo;
		shaderInfo.module = device.createShaderModule(moduleInfo);
		shaderInfo.stage = ShaderKindToVkStage(psInfo.Shader.Shaders[i].Kind);
		shaderInfo.pName = "shader";
		shaderStages.push_back(shaderInfo);
	}
	

	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;
	
	std::vector<vk::DynamicState> dynamicStates;
	dynamicStates.push_back(vk::DynamicState::eViewport);
	dynamicStates.push_back(vk::DynamicState::eScissor);
	dynamicStates.push_back(vk::DynamicState::eBlendConstants);
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssemblyInfo.primitiveRestartEnable = false;

	auto blendState = GetDefaultColorBlendState();
	auto blendattachmentState = GetDefaultColorBlendAttachmentState();
	auto dsState = GetDefaultDepthStencilstate();
	auto rasterState = GetDefaultRasterState();
	auto msState = GetDefaultMultiSampleState();
	pipelineCreateInfo.pColorBlendState = &blendState;
	pipelineCreateInfo.renderPass = nullptr;
	pipelineCreateInfo.pMultisampleState = &msState;
	pipelineCreateInfo.pDepthStencilState = &dsState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pViewportState = nullptr;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pRasterizationState = &rasterState;
	pipelineCreateInfo.pVertexInputState = &m_DefaultVertexState;
	pipelineCreateInfo.pTessellationState = nullptr;

	m_Pipeline = device.createGraphicsPipeline(nullptr,pipelineCreateInfo);


}


void PipelineState::ReloadPipelineFromFile(const vk::Device& device, const std::string& filename, vk::RenderPass rp) {
	m_DescSetLayouts.clear();
	m_ShaderBits = 0;
	m_Shaders.clear();
	device.destroyPipeline(m_Pipeline);
	device.destroyPipelineLayout(m_PipelineLayout);
	LoadPipelineFromFile(device, filename, rp);
}

void PipelineState::SetDefaultVertexState(const vk::PipelineVertexInputStateCreateInfo& vertexState) {
	m_DefaultVertexState = vertexState;
	m_DefaultVertexStateSet = true;
}

void PipelineState::SetDefaultMulitSampleState(const vk::PipelineMultisampleStateCreateInfo& msState) {
	m_DefaultMultiSampleState = msState;
	m_DefaultMultiSampleStateSet = true;
}

std::vector<vk::DescriptorSetLayout>& PipelineState::GetDescriptorSetLayouts() {
	return m_DescSetLayouts;
}

vk::Pipeline PipelineState::GetPipeline() {
	return m_Pipeline;
}

vk::PipelineLayout PipelineState::GetPipelineLayout() {
	return m_PipelineLayout;
}

