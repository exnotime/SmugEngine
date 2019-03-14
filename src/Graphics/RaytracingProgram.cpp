#if defined(RTX_ON)

#include "RaytracingProgram.h"
#include <par_shapes.h>
#include <stdio.h>
#include <glm/gtx/transform.hpp>
using namespace smug;


RaytracingProgram::RaytracingProgram() {

}

RaytracingProgram::~RaytracingProgram() {

}
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
	uint32_t       instanceOffset : 24;
	uint32_t       flags : 8;
	uint64_t       accelerationStructureHandle;
};

struct PerFrameUniforms {
	glm::mat4 invViewProj;
	glm::vec4 CamPos;
	glm::vec4 LightDir;
};

void RaytracingProgram::Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView frameBufferDesc, DeviceAllocator* allocator) {
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
	descSetCreateInfo.bindingCount = 6;
	VkDescriptorSetLayoutBinding bindings[6];
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	bindings[0].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[1].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV | VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
	bindings[2].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	bindings[3].binding = 3;
	bindings[3].descriptorCount = 1;
	bindings[3].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[3].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	bindings[4].binding = 4;
	bindings[4].descriptorCount = 1;
	bindings[4].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[4].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	bindings[5].binding = 5;
	bindings[5].descriptorCount = 1;
	bindings[5].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	bindings[5].stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	bindings[0].pImmutableSamplers = nullptr;
	bindings[1].pImmutableSamplers = nullptr;
	bindings[2].pImmutableSamplers = nullptr;
	bindings[3].pImmutableSamplers = nullptr;
	bindings[4].pImmutableSamplers = nullptr;
	bindings[5].pImmutableSamplers = nullptr;

	descSetCreateInfo.pBindings = bindings;
	vkCreateDescriptorSetLayout(device, &descSetCreateInfo, nullptr, &m_DescLayout);
	layout.setLayoutCount = 1;
	layout.pSetLayouts = &m_DescLayout;
	vkCreatePipelineLayout(device, &layout, nullptr, &m_PipelineLayout);

	pipelineInfo.layout = m_PipelineLayout;
	pipelineInfo.maxRecursionDepth = 1;

	VkShaderModuleCreateInfo moduleInfo[2];
	moduleInfo[0] = {};
	moduleInfo[0].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo[0].pNext = nullptr;
	moduleInfo[1] = {};
	moduleInfo[1].sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	moduleInfo[1].pNext = nullptr;
	LoadShaderFile("shader/spv/rg.spv", (void**)&moduleInfo[0].pCode, &(moduleInfo[0].codeSize));
	LoadShaderFile("shader/spv/ah.spv", (void**)&moduleInfo[1].pCode, &(moduleInfo[1].codeSize));
	
	VkShaderModule modules[2];
	vkCreateShaderModule(device, &moduleInfo[0], nullptr, &modules[0]);
	vkCreateShaderModule(device, &moduleInfo[1], nullptr, &modules[1]);

	VkSpecializationInfo specInfo;
	specInfo.dataSize = 0;
	specInfo.mapEntryCount = 0;
	specInfo.pData = nullptr;
	specInfo.pMapEntries = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].module = modules[0];
	shaderStages[0].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].module = modules[1];
	shaderStages[1].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pSpecializationInfo = nullptr;

	pipelineInfo.stageCount = 1;
	pipelineInfo.pStages = shaderStages;
	VkRayTracingShaderGroupCreateInfoNV groups[2];
	groups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[0].pNext = nullptr;
	groups[0].generalShader = 0;
	groups[0].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_NV;
	groups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[1].pNext = nullptr;
	groups[1].closestHitShader = 1;
	groups[1].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;

	pipelineInfo.pGroups = groups;
	pipelineInfo.groupCount = 1;
	pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

	vkCreateRayTracingPipelinesNV(device, nullptr, 1, &pipelineInfo, nullptr, &m_Pipeline);

	//create bottom level acceleration strcuture
	//create cube shape to raytrace
	par_shapes_mesh* mesh = par_shapes_create_cube();
	par_shapes_scale(mesh, 2.0f, 2.0f, 2.0f);
	par_shapes_translate(mesh, -1.0f, -1.0f, -1.0f);
	par_shapes_unweld(mesh, true);
	par_shapes_compute_normals(mesh);

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint32_t> indicies;
	for (int i = 0; i < mesh->npoints * 3; i += 3) {
		vertices.push_back(glm::vec3(mesh->points[i], mesh->points[i + 1], mesh->points[i + 2]));
		normals.push_back(glm::vec3(mesh->normals[i], mesh->normals[i + 1], mesh->normals[i + 2]));
	}
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		indicies.push_back(mesh->triangles[i + 0]);
		indicies.push_back(mesh->triangles[i + 1]);
		indicies.push_back(mesh->triangles[i + 2]);
	}

	m_InstanceBuffer = allocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, sizeof(VkGeometryInstanceNV) * 10, nullptr);
	m_Indices = allocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(uint32_t) * indicies.size(), indicies.data());
	m_VertexPositions = allocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(glm::vec3) * vertices.size(), vertices.data());
	m_VertexNormals = allocator->AllocateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(glm::vec3) * normals.size(), normals.data());
	uint32_t instanceOffsets = 0;
	//only have 1 instance right now
	m_InstanceOffsetsBuffer = allocator->AllocateBuffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(uint32_t), &instanceOffsets);

	m_blasGeo.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	m_blasGeo.geometry.triangles.indexCount = indicies.size();
	m_blasGeo.geometry.triangles.indexData = m_Indices.buffer;
	m_blasGeo.geometry.triangles.indexOffset = 0;
	m_blasGeo.geometry.triangles.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
	m_blasGeo.geometry.triangles.vertexCount = vertices.size();
	m_blasGeo.geometry.triangles.vertexData = m_VertexPositions.buffer;
	m_blasGeo.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	m_blasGeo.geometry.triangles.vertexOffset = 0;
	m_blasGeo.geometry.triangles.vertexStride = sizeof(glm::vec3);
	m_blasGeo.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	m_blasGeo.geometry.triangles.pNext = nullptr;
	m_blasGeo.geometry.triangles.transformData = nullptr;
	m_blasGeo.geometry.triangles.transformOffset = 0;
	m_blasGeo.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	m_blasGeo.geometry.aabbs.pNext = nullptr;
	m_blasGeo.geometry.aabbs.aabbData = nullptr;
	m_blasGeo.geometry.aabbs.numAABBs = 0;
	m_blasGeo.geometry.aabbs.offset = 0;
	m_blasGeo.geometry.aabbs.stride = 0;
	m_blasGeo.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
	m_blasGeo.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	m_blasGeo.pNext = nullptr;

	VkAccelerationStructureCreateInfoNV blasInfo = {};
	m_blasInfo.instanceCount = 0;
	m_blasInfo.geometryCount = 1;
	m_blasInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	m_blasInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	m_blasInfo.pNext = nullptr;
	m_blasInfo.pGeometries = &m_blasGeo;
	m_blasInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	blasInfo.info = m_blasInfo; 
	blasInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	blasInfo.pNext = nullptr;
	//create blas
	VkResult res =  vkCreateAccelerationStructureNV(device, &blasInfo, nullptr, &m_blas);
	//calc scratch buffer size
	VkAccelerationStructureMemoryRequirementsInfoNV asMemReqInfo = {};
	asMemReqInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	asMemReqInfo.pNext = nullptr;
	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	asMemReqInfo.accelerationStructure = m_blas;
	VkMemoryRequirements2KHR memReqs;
	vkGetAccelerationStructureMemoryRequirementsNV(device, &asMemReqInfo, &memReqs);

	m_ScratchBuffer = allocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, memReqs.memoryRequirements.size, nullptr, memReqs.memoryRequirements.memoryTypeBits);

	VkAccelerationStructureCreateInfoNV tlasInfo = {};
	tlasInfo.info.instanceCount = 1;
	tlasInfo.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
	tlasInfo.info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_NV;
	tlasInfo.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	tlasInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	tlasInfo.pNext = nullptr;
	vkCreateAccelerationStructureNV(device, &tlasInfo, nullptr, &m_tlas);

	//create shader binding table
	VkPhysicalDeviceRayTracingPropertiesNV rtDevProps = {};
	rtDevProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV;
	rtDevProps.pNext = nullptr;
	VkPhysicalDeviceProperties2 props = {};
	props.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	props.pNext = &rtDevProps;
	vkGetPhysicalDeviceProperties2(gpu, &props);
	m_ShaderGroupSize = rtDevProps.shaderGroupHandleSize;
	void* srtData = malloc(m_ShaderGroupSize * 2);
	vkGetRayTracingShaderGroupHandlesNV(device, m_Pipeline, 0, 1, m_ShaderGroupSize * 2, srtData);
	m_ShaderBindingTable = allocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, m_ShaderGroupSize * 2, srtData);

	m_UniformBuffer = allocator->AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameUniforms));

	//allocate descriptor set
	VkDescriptorSetAllocateInfo descAllocInfo;
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = nullptr;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &m_DescLayout;
	descAllocInfo.descriptorPool = descPool;
	vkAllocateDescriptorSets(device, &descAllocInfo, &m_DescSet);
	
	//build descriptor sets
	VkWriteDescriptorSet descWrites[6];
	VkDescriptorBufferInfo bufferDescs[4];
	VkWriteDescriptorSetAccelerationStructureNV asDescWrite = {};
	VkDescriptorImageInfo imageInfo = {};
	//framebuffer
	descWrites[0] = {};
	descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[0].pNext = nullptr;
	descWrites[0].descriptorCount = 1;
	descWrites[0].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	descWrites[0].dstArrayElement = 0;
	descWrites[0].dstSet = m_DescSet;
	descWrites[0].dstBinding = 0;
	imageInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_GENERAL;
	imageInfo.imageView = frameBufferDesc;
	imageInfo.sampler = nullptr;
	descWrites[0].pImageInfo = &imageInfo;
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
	//normals
	descWrites[3] = {};
	descWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[3].pNext = nullptr;
	descWrites[3].descriptorCount = 1;
	descWrites[3].dstArrayElement = 0;
	descWrites[3].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descWrites[3].dstSet = m_DescSet;
	descWrites[3].dstBinding = 3;
	bufferDescs[1] = {};
	bufferDescs[1].buffer = m_VertexNormals.buffer;
	bufferDescs[1].offset = 0;
	bufferDescs[1].range = VK_WHOLE_SIZE;
	descWrites[3].pBufferInfo = &bufferDescs[1];
	//indices
	descWrites[4] = {};
	descWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[4].pNext = nullptr;
	descWrites[4].descriptorCount = 1;
	descWrites[4].dstArrayElement = 0;
	descWrites[4].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descWrites[4].dstSet = m_DescSet;
	descWrites[4].dstBinding = 4;
	bufferDescs[2] = {};
	bufferDescs[2].buffer = m_Indices.buffer;
	bufferDescs[2].offset = 0;
	bufferDescs[2].range = VK_WHOLE_SIZE;
	descWrites[4].pBufferInfo = &bufferDescs[2];
	//instanceOffsets
	descWrites[5] = {};
	descWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descWrites[5].pNext = nullptr;
	descWrites[5].descriptorCount = 1;
	descWrites[5].dstArrayElement = 0;
	descWrites[5].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descWrites[5].dstSet = m_DescSet;
	descWrites[5].dstBinding = 5;
	bufferDescs[3] = {};
	bufferDescs[3].buffer = m_InstanceOffsetsBuffer.buffer;
	bufferDescs[3].offset = 0;
	bufferDescs[3].range = VK_WHOLE_SIZE;
	descWrites[5].pBufferInfo = &bufferDescs[3];
	vkUpdateDescriptorSets(device, _countof(descWrites), descWrites, 0, nullptr);
}

void RaytracingProgram::Update(CommandBuffer* cmdBuffer, DeviceAllocator* allocator) {

	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = nullptr;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	static bool first = true;
	if (first) {
		//need to build the blas the first frame
		VkAccelerationStructureInfoNV asInfo = {};
		asInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
		asInfo.geometryCount = 1;
		asInfo.pGeometries = &m_blasGeo;
		asInfo.instanceCount = 0;
		asInfo.pNext = nullptr;
		asInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		asInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;

		vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &m_blasInfo, nullptr, 0, false, m_blas, nullptr, m_ScratchBuffer.buffer, 0);
		vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
		
		//build tlas
		//transfer data
		glm::mat4x3 transform = glm::translate(glm::vec3(10, -10, 0));
		VkGeometryInstanceNV instanceData;
		instanceData.accelerationStructureHandle = *reinterpret_cast<uint64_t*>(&m_tlas);
		instanceData.flags = VkGeometryInstanceFlagBitsNV::VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
		instanceData.instanceCustomIndex = 0;
		instanceData.instanceOffset = 0;
		instanceData.mask = 0xff;
		for(int i = 0; i < 12; ++i)
			instanceData.transform[i] = (&transform[0][0])[i];

		//allocator->UpdateBuffer(m_InstanceBuffer, sizeof(instanceData), &instanceData);
		//allocator->ScheduleTransfers(cmdBuffer);

		asInfo = {};
		asInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
		asInfo.geometryCount = 0;
		asInfo.pGeometries = nullptr;
		asInfo.instanceCount = 1;
		asInfo.pNext = nullptr;
		asInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
		asInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;

		//vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &asInfo, m_InstanceBuffer.buffer, 0, false, m_tlas, nullptr, m_ScratchBuffer.buffer, 0);
		//vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

		first = false;
	}
}

void RaytracingProgram::Render(CommandBuffer* cmdBuffer) {
	return;
	vkCmdBindPipeline(cmdBuffer->CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_Pipeline);
	vkCmdBindDescriptorSets(cmdBuffer->CmdBuffer(), VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, m_PipelineLayout, 0, 1, &m_DescSet, 0, nullptr);
	vkCmdTraceRaysNV(cmdBuffer->CmdBuffer(), m_ShaderBindingTable.buffer, 0, nullptr, 0, 0, m_ShaderBindingTable.buffer, m_ShaderGroupSize, m_ShaderGroupSize, nullptr, 0, 0, 1920, 1080, 0);
}
#endif