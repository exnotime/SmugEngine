#include "VkPipeline.h"
#include <fstream>
#include <sstream>
#include <json.hpp>
#include "VkShader.h"
#include "PipelineCommon.h"
#include <AssetLoader/AssetLoader.h>
#include "Vertex.h"
using nlohmann::json;
using namespace smug;

enum SHADER_TYPES {
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	EVALUATION_SHADER,
	CONTROL_SHADER,
	COMPUTE_SHADER,
	MESH_SHADER,
	TASK_SHADER,
	RAY_GEN_SHADER,
	RAY_INTERSECTION_SHADER,
	RAY_ANY_HIT_SHADER,
	RAY_CLOSEST_HIT_SHADER,
	RAY_MISS_SHADER,
	RAY_CALLABLE_SHADER,
	SHADER_TYPE_COUNT
};

PipelineState::PipelineState() {
	m_DefaultVertexStateSet = false;
	m_DefaultMultiSampleStateSet = false;
}

PipelineState::~PipelineState() {
	m_Shaders.clear();
}

VkPipelineColorBlendAttachmentState ReadColorBlendAttachmentState(const json& blendJson) {
	VkPipelineColorBlendAttachmentState blendAttachStateInfo = GetDefaultColorBlendAttachmentState();
	if (blendJson.find("BlendEnable") != blendJson.end()) {
		blendAttachStateInfo.blendEnable = TO_VK_BOOL(blendJson["BlendEnable"]);
	}
	if (blendJson.find("SrcColorBlendFactor") != blendJson.end()) {
		std::string srcColorBlendFactor = blendJson["SrcColorBlendFactor"];
		blendAttachStateInfo.srcColorBlendFactor = ToBlendFactor[srcColorBlendFactor.c_str()];
	}
	if (blendJson.find("DstColorBlendFactor") != blendJson.end()) {
		std::string dstColorBlendFactor = blendJson["DstColorBlendFactor"];
		blendAttachStateInfo.dstColorBlendFactor = ToBlendFactor[dstColorBlendFactor.c_str()];
	}
	if (blendJson.find("ColorBlendOp") != blendJson.end()) {
		std::string colorBlendOp = blendJson["ColorBlendOp"];
		blendAttachStateInfo.colorBlendOp = ToBlendOp[colorBlendOp.c_str()];
	}
	if (blendJson.find("SrcAlphaBlendFactor") != blendJson.end()) {
		std::string srcAlphaBlendFactor = blendJson["SrcAlphaBlendFactor"];
		blendAttachStateInfo.srcAlphaBlendFactor = ToBlendFactor[srcAlphaBlendFactor.c_str()];
	}
	if (blendJson.find("DstAlphaBlendFactor") != blendJson.end()) {
		std::string dstAlphaBlendFactor = blendJson["DstAlphaBlendFactor"];
		blendAttachStateInfo.dstAlphaBlendFactor = ToBlendFactor[dstAlphaBlendFactor.c_str()];
	}
	if (blendJson.find("AlphaBlendOp") != blendJson.end()) {
		std::string alphaBlendOp = blendJson["AlphaBlendOp"];
		blendAttachStateInfo.alphaBlendOp = ToBlendOp[alphaBlendOp.c_str()];
	}
	//add color write mask
	return blendAttachStateInfo;
}

VkPipelineColorBlendStateCreateInfo ReadColorBlendState(const json& blendState) {
	VkPipelineColorBlendStateCreateInfo blendStateInfo = GetDefaultColorBlendState();
	if (blendState.find("LogicOp") != blendState.end()) {
		std::string logicOP = blendState["LogicOp"];
		blendStateInfo.logicOp = ToLogicOp[logicOP.c_str()];
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

VkPipelineDepthStencilStateCreateInfo ReadDepthStencilstate(const json& depthState) {
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = GetDefaultDepthStencilstate();
	if (depthState.find("Back") != depthState.end()) {
		std::string back = depthState["Back"];
		depthStencilCreateInfo.back.failOp = ToStencilOp[back.c_str()];
	}
	if (depthState.find("Front") != depthState.end()) {
		std::string front = depthState["Front"];
		depthStencilCreateInfo.front.failOp = ToStencilOp[front.c_str()];
	}
	if (depthState.find("DepthBoundsTestEnable") != depthState.end()) {
		depthStencilCreateInfo.depthBoundsTestEnable = TO_VK_BOOL(depthState["DepthBoundsTestEnable"]);
	}
	if (depthState.find("DepthCompareOp") != depthState.end()) {
		std::string depthCompareMode = depthState["DepthCompareOp"];
		depthStencilCreateInfo.depthCompareOp = ToCompareOp[depthCompareMode.c_str()];
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

VkPipelineMultisampleStateCreateInfo GetDefaultMultiSampleState() {
	VkPipelineMultisampleStateCreateInfo msState = {};
	msState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msState.pNext = nullptr;
	msState.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	msState.alphaToCoverageEnable = false;
	msState.alphaToOneEnable = false;
	msState.minSampleShading = 0.0f;
	msState.sampleShadingEnable = false;
	msState.pSampleMask = nullptr;
	return msState;
}

VkPipelineMultisampleStateCreateInfo ReadMultiSampleState(const json& msStateJson) {
	VkPipelineMultisampleStateCreateInfo msState = GetDefaultMultiSampleState();
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

VkPipelineRasterizationStateCreateInfo ReadRasterState(const json& rasterStateJson) {
	VkPipelineRasterizationStateCreateInfo rState = GetDefaultRasterState();
	if (rasterStateJson.find("CullMode") != rasterStateJson.end()) {
		std::string cullmode = rasterStateJson["CullMode"];
		rState.cullMode = ToCullMode[cullmode.c_str()];
	}
	if (rasterStateJson.find("FrontFace") != rasterStateJson.end()) {
		std::string frontFace = rasterStateJson["FrontFace"];
		rState.frontFace = ToFrontFace[frontFace.c_str()];
	}
	if (rasterStateJson.find("PolygonMode") != rasterStateJson.end()) {
		std::string polygonMode = rasterStateJson["PolygonMode"];
		rState.polygonMode = ToPolygonMode[polygonMode.c_str()];
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

void PipelineState::LoadPipelineFromFile(const VkDevice& device, const eastl::string& filename, VkRenderPass renderPass) {
	std::ifstream fin(filename.c_str());
	if (!fin.is_open()) {
		printf("Error opening pipeline file %s\n", filename.c_str());
		fin.close();
		return;
	}
	json root;
	try {
		fin >> root;
	}
	catch (std::exception e) {
		printf("json: %s\n", e.what());
		return;
	}
	fin.close();
	//Find shaders
	m_ShaderBits = 0;
	if (root.find("Shaders") != root.end()) {
		json shaders = root["Shaders"];

		const char* shader_types[] = { "Vertex", "Fragment", "Geometry", "EVALUATION", "Control", "Compute", "Mesh", "Task", "RayGen", "RayInt", "RayAnyHit", "RayClosestHit", "RayMiss", "RayCall" };
		for (int i = 0; i < SHADER_TYPE_COUNT; ++i) {
			if (shaders.find(shader_types[i]) != shaders.end()) {
				json shader = shaders[shader_types[i]];
				eastl::string entry;
				SHADER_LANGUAGE lang = GLSL;
				if (shader.find("EntryPoint") != shader.end()) {
					std::string entry_point = shader["EntryPoint"];
					entry = entry_point.c_str();
				}
				else {
					entry = "main";
				}
				if (shader.find("Language") != shader.end()) {
					if (shader["Language"] == "GLSL") {
						lang = GLSL;
					}
					else if (shader["Language"] == "HLSL") {
						lang = HLSL;
					}
				}
				std::string source = shader["Source"];
				m_Shaders.push_back(LoadShaderModule(device, source.c_str(), SHADER_KIND(i), entry, lang));
				m_ShaderBits |= 1 << i;
			}
			else {
				m_Shaders.push_back(VkShaderModule());
			}
		}
	}
	else {
		return;
	}
	//descriptor set
	if (root.find("DescriptorSetLayouts") != root.end()) {
		json descSetLayouts = root["DescriptorSetLayouts"];
		for (auto& layout : descSetLayouts) {
			eastl::vector<VkDescriptorSetLayoutBinding> bindings;

			for (auto& bind : layout) {
				VkDescriptorSetLayoutBinding binding = {};
				binding.binding = bind["Binding"];
				binding.descriptorCount = bind["Count"];
				
				std::string type = bind["Type"];
				binding.descriptorType = ToDescriptorType[type.c_str()];
				binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL; //keep the stage set to all for now
				bindings.push_back(binding);
			}
			VkDescriptorSetLayoutCreateInfo descSetCreateInfo = {};
			descSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetCreateInfo.pNext = nullptr;
			descSetCreateInfo.bindingCount = (uint32_t)bindings.size();
			descSetCreateInfo.pBindings = bindings.data();
			VkDescriptorSetLayout layout;
			vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, &layout);
			m_DescSetLayouts.push_back(layout);
		}
	}
	//push constants
	eastl::vector<VkPushConstantRange> pushConstRanges;
	if (root.find("PushConstantRanges") != root.end()) {
		json pushConstants = root["PushConstantRanges"];
		for (auto& pc : pushConstants) {
			VkPushConstantRange pushConst;
			pushConst.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL; //keep the stage set to all for now
			pushConst.size = pc["Size"];
			pushConst.offset = pc["Offset"];
			pushConstRanges.push_back(pushConst);
		}
	}
	//pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.pNext = nullptr;
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstRanges.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = (uint32_t)pushConstRanges.size();
	pipelineLayoutCreateInfo.pSetLayouts = m_DescSetLayouts.data();
	pipelineLayoutCreateInfo.setLayoutCount = (uint32_t)m_DescSetLayouts.size();
	vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout);

	//if we use a compute shader we dont need anything else
	if (m_ShaderBits & (1 << COMPUTE_SHADER)) {
		VkComputePipelineCreateInfo pipelineInfo;
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.layout = m_PipelineLayout;
		VkPipelineShaderStageCreateInfo computeStage;
		computeStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		computeStage.pNext = nullptr;
		computeStage.module = m_Shaders[0];
		computeStage.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		computeStage.pName = "main";
		pipelineInfo.stage = computeStage;
		vkCreateComputePipelines(device, nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline);
		return;
	}

	//color blend state
	VkPipelineColorBlendStateCreateInfo blendStateInfo = {};
	blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateInfo.pNext = nullptr;
	eastl::vector<VkPipelineColorBlendAttachmentState> ColorblendAttachmentStates;
	if (root.find("ColorBlendState") != root.end()) {
		json ColorBlendStateJson = root["ColorBlendState"];
		blendStateInfo = ReadColorBlendState(ColorBlendStateJson);
		//blend attachments
		if (ColorBlendStateJson.find("ColorBlendAttachmentStates") != ColorBlendStateJson.end()) {
			json blendAttachmentStatesJson = ColorBlendStateJson["ColorBlendAttachmentStates"];
			for (auto& state : blendAttachmentStatesJson) {
				ColorblendAttachmentStates.push_back(ReadColorBlendAttachmentState(state));
			}
		}
		else {
			ColorblendAttachmentStates.push_back(GetDefaultColorBlendAttachmentState());
		}
	}
	else {
		blendStateInfo = GetDefaultColorBlendState();
	}
	blendStateInfo.attachmentCount = (uint32_t)ColorblendAttachmentStates.size();
	blendStateInfo.pAttachments = ColorblendAttachmentStates.data();
	//depth stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilState;
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.pNext = nullptr;
	if (root.find("DepthStencilState") != root.end()) {
		json depthStencilJson = root["DepthStencilState"];
		depthStencilState = ReadDepthStencilstate(depthStencilJson);
	}
	else {
		depthStencilState = GetDefaultDepthStencilstate();
	}
	//input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.pNext = nullptr;
	if (root.find("InputAssemblyState") != root.end()) {
		json inputAssemJson = root["InputAssemblyState"];
		if (inputAssemJson.find("PrimitiveRestartEnable") != inputAssemJson.end()) {
			inputAssemblyInfo.primitiveRestartEnable = TO_VK_BOOL(inputAssemJson["PrimitiveRestartEnable"]);
		}
		else {
			inputAssemblyInfo.primitiveRestartEnable = false;
		}
		if (inputAssemJson.find("Topology") != inputAssemJson.end()) {
			std::string topology = inputAssemJson["Topology"];
			inputAssemblyInfo.topology = ToPrimitiveTopology[topology.c_str()];
		}
		else {
			inputAssemblyInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}
	else {
		inputAssemblyInfo.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = false;
	}
	// ms state
	VkPipelineMultisampleStateCreateInfo* multisampleStateInfo = nullptr;
	if (root.find("MultiSampleState") != root.end()) {
		json msStateJson = root["MultiSampleState"];
		multisampleStateInfo = new VkPipelineMultisampleStateCreateInfo();
		*multisampleStateInfo = ReadMultiSampleState(msStateJson);
	}
	else if (m_DefaultMultiSampleStateSet) {
		// do nothing
	}
	else {
		multisampleStateInfo = new VkPipelineMultisampleStateCreateInfo();
		*multisampleStateInfo = GetDefaultMultiSampleState();
	}
	//raster state
	VkPipelineRasterizationStateCreateInfo rasterInfo;
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.pNext = nullptr;
	if (root.find("RasterizationState") != root.end()) {
		json rsStateJson = root["RasterizationState"];
		rasterInfo = ReadRasterState(rsStateJson);
	}
	else {
		rasterInfo = GetDefaultRasterState();
	}
	//tesselation state
	VkPipelineTessellationStateCreateInfo* tesselationstate = nullptr;
	if (root.find("TesselationState") != root.end()) {
		json tStateJson = root["TesselationState"];
		tesselationstate = new VkPipelineTessellationStateCreateInfo();
		if (tStateJson.find("PatchControlPoints") != tStateJson.end()) {
			tesselationstate->patchControlPoints = tStateJson["PatchControlPoints"];
		}
		else {
			tesselationstate->patchControlPoints = 1;
		}
	}
	//shader stages
	eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (int i = 0; i < 6; i++) {
		VkPipelineShaderStageCreateInfo stage = {};
		stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stage.pNext = nullptr;
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
	VkPipelineVertexInputStateCreateInfo* vertexState = nullptr;
	eastl::vector<VkVertexInputBindingDescription> vertexBindings;
	eastl::vector<VkVertexInputAttributeDescription> vertexInputDescs;
	if (root.find("VertexInputState") != root.end()) {
		json vertexStateJson = root["VertexInputState"];
		//bindings
		if (vertexStateJson.find("InputBindings") != vertexStateJson.end()) {
			json bindings = vertexStateJson["InputBindings"];
			for (auto& bind : bindings) {
				VkVertexInputBindingDescription bindDesc;
				bindDesc.binding = bind["Binding"];
				std::string inputRate = bind["InputRate"];
				bindDesc.inputRate = ToVertexInputRate[inputRate.c_str()];
				bindDesc.stride = bind["Stride"];
				vertexBindings.push_back(bindDesc);
			}
		}
		//input attributes
		if (vertexStateJson.find("InputAttributes") != vertexStateJson.end()) {
			json attributes = vertexStateJson["InputAttributes"];
			for (auto& attr : attributes) {
				VkVertexInputAttributeDescription attrDesc;
				attrDesc.binding = attr["Binding"];
				std::string format = attr["Format"];
				attrDesc.format = ToFormat[format.c_str()];
				attrDesc.location = attr["Location"];
				attrDesc.offset = attr["Offset"];
				vertexInputDescs.push_back(attrDesc);
			}
		}
		vertexState = new VkPipelineVertexInputStateCreateInfo();
		vertexState->pVertexBindingDescriptions = vertexBindings.data();
		vertexState->vertexBindingDescriptionCount = (uint32_t)vertexBindings.size();
		vertexState->pVertexAttributeDescriptions = vertexInputDescs.data();
		vertexState->vertexAttributeDescriptionCount = (uint32_t)vertexInputDescs.size();
	}

	//viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
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

	eastl::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VkDynamicState::VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	//put everything together
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
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

	vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline);

	//clean up
	if (vertexState) delete vertexState;
	if (multisampleStateInfo) delete multisampleStateInfo;
	if (tesselationstate) delete tesselationstate;
}

VkShaderStageFlagBits ShaderKindToVkStage(SHADER_KIND k) {
	switch (k)
	{
	case smug::VERTEX:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case smug::FRAGMENT:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case smug::GEOMETRY:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case smug::EVALUATION:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case smug::CONTROL:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case smug::COMPUTE:
		return VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	}
	return VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;

}

void PipelineState::LoadPipelineFromInfo(const VkDevice& device, const PipelineStateInfo& psInfo, VkRenderPass rp) {
	VkPipelineLayoutCreateInfo layoutCreateInfo = {};
	eastl::vector<VkDescriptorSetLayout> setLayouts;
	eastl::unordered_map<uint32_t, eastl::vector<VkDescriptorSetLayoutBinding>> bindings;

	for (uint32_t i = 0; i < psInfo.DescriptorCount; ++i) {
		Descriptor& desc = psInfo.Descriptors[i];
		VkDescriptorSetLayoutBinding bind = {};
		bind.binding = desc.Bindpoint;
		bind.descriptorCount = desc.Count;
		bind.descriptorType = (VkDescriptorType)desc.Type;
		bind.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		bind.pImmutableSamplers = nullptr;
		bool found = false;
		for (auto& desc : bindings[desc.Set]) {
			if (desc.binding == bind.binding) {
				found = true;
				break;
			}
		}
		if(!found)
			bindings[desc.Set].push_back(bind);
	}
	m_DescSetLayouts.resize(bindings.size());
	for (uint32_t i = 0; i < bindings.size(); ++i) {
		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.pNext = nullptr;
		info.bindingCount = (uint32_t)bindings[i].size();
		info.pBindings = bindings[i].data();
		vkCreateDescriptorSetLayout(device, &info, nullptr, &m_DescSetLayouts[i]);
	}
	layoutCreateInfo.pSetLayouts = m_DescSetLayouts.data();
	layoutCreateInfo.setLayoutCount = (uint32_t)m_DescSetLayouts.size();
	
	if (psInfo.PushConstants.Size > 0) {
		layoutCreateInfo.pushConstantRangeCount = 1;
		VkPushConstantRange pc;
		pc.offset = psInfo.PushConstants.Offset;
		pc.size = psInfo.PushConstants.Size;
		pc.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
		layoutCreateInfo.pPushConstantRanges = &pc;
	}
	layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutCreateInfo.pNext = nullptr;
	vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &m_PipelineLayout);


	eastl::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	for (uint32_t i = 0; i < psInfo.ShaderCount; ++i) {
		VkShaderModuleCreateInfo moduleInfo = {};
		moduleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleInfo.pNext = nullptr;
		moduleInfo.codeSize = psInfo.Shaders[i].ByteCodeSize;
		moduleInfo.pCode = (uint32_t*)psInfo.Shaders[i].ByteCode;
		VkPipelineShaderStageCreateInfo shaderInfo = {};
		shaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderInfo.pNext = nullptr;
		vkCreateShaderModule(device, &moduleInfo, nullptr, &shaderInfo.module);
		shaderInfo.stage = ShaderKindToVkStage(psInfo.Shaders[i].Kind);
		shaderInfo.pName = "main";
		shaderStages.push_back(shaderInfo);
	}
	
	if (psInfo.Shaders[0].Kind == COMPUTE) {
		VkComputePipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = nullptr;
		pipelineInfo.layout = m_PipelineLayout;
		pipelineInfo.stage = shaderStages[0];
		vkCreateComputePipelines(device, nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline);
		return;
	}

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.pNext = nullptr;
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = 0;
	
	eastl::vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VkDynamicState::VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VkDynamicState::VK_DYNAMIC_STATE_SCISSOR);
	dynamicStates.push_back(VkDynamicState::VK_DYNAMIC_STATE_BLEND_CONSTANTS);
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {};
	dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateCreateInfo.pNext = nullptr;
	dynamicStateCreateInfo.dynamicStateCount = (uint32_t)dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.topology = psInfo.Topology;
	inputAssemblyInfo.primitiveRestartEnable = false;

	VkPipelineColorBlendStateCreateInfo blendState = GetDefaultColorBlendState();
	blendState.attachmentCount = psInfo.AttachmentCount;
	blendState.pAttachments = psInfo.Attachments;

	auto msState = GetDefaultMultiSampleState();

	VkPipelineVertexInputStateCreateInfo vertexState = Geometry::GetVertexState();
	
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.pNext = nullptr;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.pScissors = nullptr;

	pipelineCreateInfo.pColorBlendState = &blendState;
	pipelineCreateInfo.renderPass = rp;
	pipelineCreateInfo.pMultisampleState = &msState;
	pipelineCreateInfo.pDepthStencilState = &psInfo.DepthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pRasterizationState = &psInfo.RasterState;
	pipelineCreateInfo.pVertexInputState = &vertexState;
	pipelineCreateInfo.pTessellationState = nullptr;

	vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline);
}


void PipelineState::ReloadPipelineFromFile(const VkDevice& device, const eastl::string& filename, VkRenderPass rp) {
	m_DescSetLayouts.clear();
	m_ShaderBits = 0;
	m_Shaders.clear();
	vkDestroyPipeline(device,m_Pipeline,nullptr);
	vkDestroyPipelineLayout(device, m_PipelineLayout, nullptr);
	LoadPipelineFromFile(device, filename, rp);
}

void PipelineState::SetDefaultVertexState(const VkPipelineVertexInputStateCreateInfo& vertexState) {
	m_DefaultVertexState = vertexState;
	m_DefaultVertexStateSet = true;
}

void PipelineState::SetDefaultMulitSampleState(const VkPipelineMultisampleStateCreateInfo& msState) {
	m_DefaultMultiSampleState = msState;
	m_DefaultMultiSampleStateSet = true;
}

const eastl::vector<VkDescriptorSetLayout>& PipelineState::GetDescriptorSetLayouts() const {
	return m_DescSetLayouts;
}

VkPipeline PipelineState::GetPipeline() const  {
	return m_Pipeline;
}

VkPipelineLayout PipelineState::GetPipelineLayout() const  {
	return m_PipelineLayout;
}

