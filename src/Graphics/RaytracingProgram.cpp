#if defined(RTX_ON)

#include "RaytracingProgram.h"
#include "VkShader.h"
#include "GraphicsObjects.h"
#include <Core/Camera.h>
#include <par_shapes.h>
#include <stdio.h>
#include <glm/gtx/transform.hpp>
#include <EASTL/unordered_map.h>
using namespace smug;

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

struct VkGeometryInstanceNV {
	float          transform[12];
	uint32_t       instanceCustomIndex : 24;
	uint32_t       mask : 8;
	uint32_t       hitGroupIndex : 24;
	uint32_t       flags : 8;
	uint64_t       accelerationStructureHandle;
};

struct PerFrameUniforms {
	glm::mat4 invViewProj;
	glm::vec4 CamPos;
	glm::vec4 LightDir;
};

void RaytracingProgram::Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView outputTarget, VkImageView depthStencil, VkImageView normals, DeviceAllocator& allocator) {
	VkRayTracingPipelineCreateInfoNV pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.basePipelineHandle = nullptr;
	pipelineInfo.basePipelineIndex = 0;
	VkPipelineLayoutCreateInfo layout = {};
	layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout.pNext = nullptr;
	layout.pushConstantRangeCount = 0;
	layout.pPushConstantRanges = nullptr;
	VkDescriptorSetLayoutCreateInfo descSetCreateInfo = {};
	descSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descSetCreateInfo.pNext = nullptr;
	descSetCreateInfo.bindingCount = 5;
	VkDescriptorSetLayoutBinding bindings[5];
	int c = 0;
	bindings[c].binding = 0;
	bindings[c].descriptorCount = 1;
	bindings[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[c++].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[c].binding = 1;
	bindings[c].descriptorCount = 1;
	bindings[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[c++].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV | VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	bindings[c].binding = 2;
	bindings[c].descriptorCount = 1;
	bindings[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	bindings[c++].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[c].binding = 3;
	bindings[c].descriptorCount = 1;
	bindings[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[c++].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[c].binding = 4;
	bindings[c].descriptorCount = 1;
	bindings[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[c++].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[3].pImmutableSamplers = nullptr;
	bindings[4].pImmutableSamplers = nullptr;

	descSetCreateInfo.pBindings = bindings;
	vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, &m_DescLayout);
	layout.setLayoutCount = 1;
	layout.pSetLayouts = &m_DescLayout;
	vkCreatePipelineLayout(device, &layout, nullptr, &m_PipelineLayout);

	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.maxRecursionDepth = 1;

	VkSpecializationInfo specInfo;
	specInfo.dataSize = 0;
	specInfo.mapEntryCount = 0;
	specInfo.pData = nullptr;
	specInfo.pMapEntries = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[4];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].module = LoadShaderModule(device, "shader/RayGen.glsl", SHADER_KIND::RAY_GEN);
	shaderStages[0].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].module = LoadShaderModule(device, "shader/RayClosestHit.glsl", SHADER_KIND::RAY_CLOSEST_HIT);
	shaderStages[1].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pSpecializationInfo = nullptr;
	shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[2].pNext = nullptr;
	shaderStages[2].module = LoadShaderModule(device, "shader/RayAnyHit.glsl", SHADER_KIND::RAY_ANY_HIT);
	shaderStages[2].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_NV;
	shaderStages[2].pName = "main";
	shaderStages[2].flags = 0;
	shaderStages[2].pSpecializationInfo = nullptr;
	shaderStages[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[3].pNext = nullptr;
	shaderStages[3].module = LoadShaderModule(device, "shader/RayMiss.glsl", SHADER_KIND::RAY_MISS);
	shaderStages[3].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_NV;
	shaderStages[3].pName = "main";
	shaderStages[3].flags = 0;
	shaderStages[3].pSpecializationInfo = nullptr;

	pipelineInfo.stageCount = _countof(shaderStages);
	pipelineInfo.pStages = shaderStages;
	VkRayTracingShaderGroupCreateInfoNV groups[3];
	//ray gen
	groups[0] = {};
	groups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[0].pNext = nullptr;
	groups[0].generalShader = 0;
	groups[0].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[0].closestHitShader = VK_SHADER_UNUSED_NV;
	groups[0].intersectionShader = VK_SHADER_UNUSED_NV;
	groups[0].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	//hit group
	groups[1] = {};
	groups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[1].pNext = nullptr;
	groups[1].closestHitShader = 1;
	groups[1].generalShader = VK_SHADER_UNUSED_NV;
	groups[1].anyHitShader = 2;
	groups[1].intersectionShader = VK_SHADER_UNUSED_NV;
	groups[1].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	//miss
	groups[2] = {};
	groups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[2].pNext = nullptr;
	groups[2].generalShader = 3;
	groups[2].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[2].closestHitShader = VK_SHADER_UNUSED_NV;
	groups[2].intersectionShader = VK_SHADER_UNUSED_NV;
	groups[2].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	

	pipelineInfo.pGroups = groups;
	pipelineInfo.groupCount = _countof(groups);
	pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	VkResult res = vkCreateRayTracingPipelinesNV(device, nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline);

	//create shader binding table
	VkPhysicalDeviceRayTracingPropertiesNV rtDevProps = {};
	rtDevProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
	rtDevProps.pNext = nullptr;
	VkPhysicalDeviceProperties2KHR props = {};
	props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	props.pNext = &rtDevProps;
	vkGetPhysicalDeviceProperties2KHR(gpu, &props);
	m_ShaderGroupSize = rtDevProps.shaderGroupHandleSize;
	void* srtData = malloc(m_ShaderGroupSize * 3);
	vkGetRayTracingShaderGroupHandlesNV(device, m_Pipeline, 0, 3, m_ShaderGroupSize * 3, srtData);
	m_ShaderBindingTable = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, m_ShaderGroupSize * 3, srtData);
	free(srtData);
	m_InstanceBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, sizeof(VkGeometryInstanceNV) * MAX_INSTANCES * 2, nullptr);
	
	//create tlas
	VkAccelerationStructureCreateInfoNV tlasInfo = {};
	m_tlasInfo.instanceCount = MAX_INSTANCES * 2; //potential maximum of static + dynamic render queue
	m_tlasInfo.geometryCount = 0;
	m_tlasInfo.pGeometries = nullptr;
	m_tlasInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	m_tlasInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
	m_tlasInfo.pNext = nullptr;
	m_tlasInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	tlasInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	tlasInfo.pNext = nullptr;
	tlasInfo.info = m_tlasInfo;
	res = vkCreateAccelerationStructureNV(device, &tlasInfo, nullptr, &m_tlas);

	VkAccelerationStructureMemoryRequirementsInfoNV asMemReqInfo = {};
	asMemReqInfo.accelerationStructure = m_tlas;
	asMemReqInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	asMemReqInfo.pNext = nullptr;
	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;

	VkMemoryRequirements2KHR memReqs = {};
	memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
	memReqs.pNext = &asMemReqInfo;

	vkGetAccelerationStructureMemoryRequirementsNV(device, &asMemReqInfo, &memReqs);
	m_TLASBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, memReqs.memoryRequirements.size, nullptr, memReqs.memoryRequirements.memoryTypeBits);

	VkBindAccelerationStructureMemoryInfoNV tlasBindInfo = {};
	tlasBindInfo.accelerationStructure = m_tlas;
	tlasBindInfo.memory = allocator.GetMemory(m_TLASBuffer.memory);
	tlasBindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	tlasBindInfo.pNext = nullptr;
	tlasBindInfo.memoryOffset = allocator.GetMemoryOffset(m_TLASBuffer.memory);
	tlasBindInfo.deviceIndexCount = 0;
	tlasBindInfo.pDeviceIndices = nullptr;
	res = vkBindAccelerationStructureMemoryNV(device, 1, &tlasBindInfo);

	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	vkGetAccelerationStructureMemoryRequirementsNV(device, &asMemReqInfo, &memReqs);

	m_ScratchBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, memReqs.memoryRequirements.size, nullptr, memReqs.memoryRequirements.memoryTypeBits);
	m_UniformBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameUniforms));

	//allocate descriptor set
	VkDescriptorSetAllocateInfo descAllocInfo;
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = nullptr;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &m_DescLayout;
	descAllocInfo.descriptorPool = descPool;
	vkAllocateDescriptorSets(device, &descAllocInfo, &m_DescSet);
	
	//build descriptor sets
	VkWriteDescriptorSet descWrites[5];
	VkDescriptorBufferInfo bufferDescs[1];
	VkWriteDescriptorSetAccelerationStructureNV asDescWrite = {};
	VkDescriptorImageInfo imageInfo[3];
	//framebuffer
	descWrites[0] = {};
	descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[0].pNext = nullptr;
	descWrites[0].descriptorCount = 1;
	descWrites[0].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descWrites[0].dstArrayElement = 0;
	descWrites[0].dstSet = m_DescSet;
	descWrites[0].dstBinding = 0;
	imageInfo[0].imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
	imageInfo[0].imageView = outputTarget;
	imageInfo[0].sampler = nullptr;
	descWrites[0].pImageInfo = &imageInfo[0];
	//uniform buffer
	descWrites[1] = {};
	descWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[1].pNext = nullptr;
	descWrites[1].descriptorCount = 1;
	descWrites[1].dstArrayElement = 0;
	descWrites[1].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descWrites[1].dstSet = m_DescSet;
	descWrites[1].dstBinding = 1;
	bufferDescs[0] = {};
	bufferDescs[0].buffer = m_UniformBuffer.buffer;
	bufferDescs[0].offset = 0;
	bufferDescs[0].range = VK_WHOLE_SIZE;
	descWrites[1].pBufferInfo = &bufferDescs[0];
	//AccelerationStructure
	descWrites[2] = {};
	descWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[2].descriptorCount = 1;
	descWrites[2].dstArrayElement = 0;
	descWrites[2].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	descWrites[2].dstSet = m_DescSet;
	descWrites[2].dstBinding = 2;
	asDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
	asDescWrite.pNext = nullptr;
	asDescWrite.accelerationStructureCount = 1;
	asDescWrite.pAccelerationStructures = reinterpret_cast<VkAccelerationStructureNV*>(&m_tlas);
	descWrites[2].pNext = &asDescWrite;
	//depth stencil
	VkSamplerCreateInfo sampInfo = {};
	sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampInfo.pNext = nullptr;
	sampInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.anisotropyEnable = false;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
	sampInfo.magFilter = VkFilter::VK_FILTER_NEAREST;
	sampInfo.minFilter = VkFilter::VK_FILTER_NEAREST;
	sampInfo.maxLod = 1.0f;
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampInfo.unnormalizedCoordinates = false;
	vkCreateSampler(device, &sampInfo, nullptr, &m_Sampler);

	descWrites[3] = {};
	descWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[3].pNext = nullptr;
	descWrites[3].descriptorCount = 1;
	descWrites[3].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[3].dstArrayElement = 0;
	descWrites[3].dstSet = m_DescSet;
	descWrites[3].dstBinding = 3;
	imageInfo[1].imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo[1].imageView = depthStencil;
	imageInfo[1].sampler = m_Sampler;
	descWrites[3].pImageInfo = &imageInfo[1];

	descWrites[4] = {};
	descWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[4].pNext = nullptr;
	descWrites[4].descriptorCount = 1;
	descWrites[4].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[4].dstArrayElement = 0;
	descWrites[4].dstSet = m_DescSet;
	descWrites[4].dstBinding = 4;
	imageInfo[2].imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo[2].imageView = normals;
	imageInfo[2].sampler = m_Sampler;
	descWrites[4].pImageInfo = &imageInfo[2];

	vkUpdateDescriptorSets(device, _countof(descWrites), descWrites, 0, nullptr);
}

void RaytracingProgram::AddRenderQueueToTLAS(RenderQueue* queue, bool staticRenderQueue, DeviceAllocator& allocator, ResourceHandler& resources) {
	uint32_t instanceCount = queue->GetInputs().size();
	eastl::vector<VkGeometryInstanceNV> instances;

	const eastl::unordered_map<ResourceHandle, ModelInstance>& modelsQueue = queue->GetModels();
	uint32_t offset = staticRenderQueue ? 0 : m_StaticInstanceCount;
	uint32_t actualInstanceCount = 0;
	for (const auto& modelGroup : modelsQueue) {
		for (uint32_t i = 0; i < modelGroup.second.Count; ++i) {
			const Model& model = resources.GetModel(modelGroup.first);
			instances.resize(instances.size() + model.MeshCount);
			for (uint32_t m = 0; m < model.MeshCount; ++m) {
				const Mesh& mesh = model.Meshes[m];
				const BLAS& blas = mesh.Blas;
				if (blas.Built) {
					VkGeometryInstanceNV& inst = instances[actualInstanceCount];
					inst.accelerationStructureHandle = blas.Handle;
					inst.flags = blas.opaque ? VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV : VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_NV;
					inst.hitGroupIndex = blas.opaque ? 0 : 1; //TODO! Add index to special geometry hit shader here
					inst.instanceCustomIndex = offset + i;
					inst.mask = 0xff;
					memcpy(inst.transform, &modelGroup.second.Inputs[i].Transform, sizeof(glm::mat4x3));
					actualInstanceCount++;
				}
			}
			
		}
		offset += modelGroup.second.Count;
	}

	if (staticRenderQueue) {
		m_StaticInstanceCount = actualInstanceCount;
		allocator.UpdateBuffer(m_InstanceBuffer, actualInstanceCount * sizeof(VkGeometryInstanceNV), instances.data());
		m_tlasInfo.instanceCount = m_StaticInstanceCount;
	} else {
		allocator.UpdateBuffer(m_InstanceBuffer, actualInstanceCount * sizeof(VkGeometryInstanceNV), instances.data(), m_StaticInstanceCount * sizeof(VkGeometryInstanceNV));
		m_tlasInfo.instanceCount = m_StaticInstanceCount + actualInstanceCount;
	}
}

void RaytracingProgram::BuildTLAS(CommandBuffer* cmdBuffer) {
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = nullptr;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &m_tlasInfo, m_InstanceBuffer.buffer, 0, false, m_tlas, nullptr, m_ScratchBuffer.buffer, 0);
	vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

void RaytracingProgram::Update(DeviceAllocator* allocator, const CameraData* cd, const glm::vec3& lightDir) {
	PerFrameUniforms ub;
	ub.invViewProj = glm::inverse(cd->ProjView);
	ub.CamPos = glm::vec4(cd->Position, 1);
	ub.LightDir = glm::vec4(lightDir, 1);

	allocator->UpdateBuffer(m_UniformBuffer, sizeof(PerFrameUniforms), &ub);

}

void RaytracingProgram::Render(CommandBuffer* cmdBuffer) {
	vkCmdBindPipeline(cmdBuffer->CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_Pipeline);
	vkCmdBindDescriptorSets(cmdBuffer->CmdBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_PipelineLayout, 0, 1, &m_DescSet, 0, nullptr);
	vkCmdTraceRaysNV(cmdBuffer->CmdBuffer(),
		m_ShaderBindingTable.buffer, 0, //raygen
		m_ShaderBindingTable.buffer, m_ShaderGroupSize * 2, m_ShaderGroupSize, //miss
		m_ShaderBindingTable.buffer, m_ShaderGroupSize, m_ShaderGroupSize, //hitgroup
		nullptr, 0, 0, 1920, 1080, 1);
}

void RaytracingProgram::DeInit(DeviceAllocator* allocator) {
	allocator->DeAllocateBuffer(m_InstanceBuffer);
	allocator->DeAllocateBuffer(m_UniformBuffer);
	allocator->DeAllocateBuffer(m_ScratchBuffer);
	allocator->DeAllocateBuffer(m_TLASBuffer);
	allocator->DeAllocateBuffer(m_ShaderBindingTable);
}


#endif