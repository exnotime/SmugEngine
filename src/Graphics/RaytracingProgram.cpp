#include "RaytracingProgram.h"
#include <stdio.h>
using namespace smug;


RaytracingProgram::RaytracingProgram() {

}

RaytracingProgram::~RaytracingProgram() {

}
#if 0
void LoadShaderFile(const char* file, void** code, size_t* codeSize) {
	FILE* f = fopen(file, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		*codeSize = ftell(f);
		rewind(f);
		*code = malloc(*codeSize);
		fread(*code, sizeof(char), *codeSize, f);
		fclose(f);
	}
}

void RaytracingProgram::Init(vk::Device device, vk::Instance instance) {

	vk::RayTracingPipelineCreateInfoNV pipelineInfo;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = 0;
	vk::PipelineLayoutCreateInfo layout;
	layout.pushConstantRangeCount = 0;
	layout.pPushConstantRanges = nullptr;
	vk::DescriptorSetLayoutCreateInfo descSetCreateInfo;
	descSetCreateInfo.bindingCount = 5;
	vk::DescriptorSetLayoutBinding bindings[5];
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	bindings[0].stageFlags = vk::ShaderStageFlagBits::eAnyHitNV | vk::ShaderStageFlagBits::eRaygenNV;
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = vk::DescriptorType::eAccelerationStructureNV;
	bindings[1].stageFlags = vk::ShaderStageFlagBits::eRaygenNV;
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[2].stageFlags = vk::ShaderStageFlagBits::eAnyHitNV;
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[3].stageFlags = vk::ShaderStageFlagBits::eAnyHitNV;
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = vk::DescriptorType::eStorageBuffer;
	bindings[4].stageFlags = vk::ShaderStageFlagBits::eAnyHitNV;
	descSetCreateInfo.pBindings = bindings;
	m_DescLayout = device.createDescriptorSetLayout(descSetCreateInfo);
	layout.setLayoutCount = 1;
	layout.pSetLayouts = &m_DescLayout;
	m_PipelineLayout = device.createPipelineLayout(layout);

	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.maxRecursionDepth = 2;

	vk::ShaderModuleCreateInfo moduleInfo[2];
	LoadShaderFile("shader/spv/rg.spv", (void**)&moduleInfo[0].pCode, &(moduleInfo[0].codeSize));
	LoadShaderFile("shader/spv/ah.spv", (void**)&moduleInfo[1].pCode, &(moduleInfo[1].codeSize));

	vk::ShaderModule modules[2];
	modules[0] = device.createShaderModule(moduleInfo[0]);
	modules[1] = device.createShaderModule(moduleInfo[1]);
	vk::SpecializationInfo specInfo;
	specInfo.dataSize = 0;
	specInfo.mapEntryCount = 0;
	specInfo.pData = nullptr;
	specInfo.pMapEntries = nullptr;
	vk::PipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].module = modules[0];
	shaderStages[0].stage = vk::ShaderStageFlagBits::eRaygenNV;
	shaderStages[0].pName = "RayGen";
	shaderStages[0].pSpecializationInfo = &specInfo;
	shaderStages[1].module = modules[1];
	shaderStages[1].stage = vk::ShaderStageFlagBits::eAnyHitNV;
	shaderStages[1].pName = "AnyHit";
	shaderStages[1].pSpecializationInfo = &specInfo;

	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	vk::RayTracingShaderGroupCreateInfoNV groups;
	groups.anyHitShader = 0;
	pipelineInfo.pGroups = &groups;

	m_Pipeline = device.createRayTracingPipelineNV(nullptr, pipelineInfo);

}

void RaytracingProgram::Render(CommandBuffer* cmdBuffer) {

}
#endif