#if defined(RTX_ON)

#include "RaytracingProgram.h"
#include "VkShader.h"
#include "GraphicsObjects.h"
#include <Core/Camera.h>
#include <par_shapes.h>
#include <stdio.h>
#include <glm/gtx/transform.hpp>
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

void RaytracingProgram::Init(VkDevice device, VkPhysicalDevice gpu, VkInstance instance, VkDescriptorPool descPool, VkImageView frameBufferDesc, DeviceAllocator& allocator) {
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

	VkSpecializationInfo specInfo;
	specInfo.dataSize = 0;
	specInfo.mapEntryCount = 0;
	specInfo.pData = nullptr;
	specInfo.pMapEntries = nullptr;

	VkPipelineShaderStageCreateInfo shaderStages[3];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].module = LoadShader(device, "shader/RayGen.glsl", SHADER_KIND::RAY_GEN);
	shaderStages[0].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_NV;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].module = LoadShader(device, "shader/RayClosestHit.glsl", SHADER_KIND::RAY_CLOSEST_HIT);
	shaderStages[1].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pSpecializationInfo = nullptr;
	shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[2].pNext = nullptr;
	shaderStages[2].module = LoadShader(device, "shader/RayMiss.glsl", SHADER_KIND::RAY_MISS);
	shaderStages[2].stage = VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_NV;
	shaderStages[2].pName = "main";
	shaderStages[2].flags = 0;
	shaderStages[2].pSpecializationInfo = nullptr;

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
	//closest hit
	groups[1] = {};
	groups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[1].pNext = nullptr;
	groups[1].closestHitShader = 1;
	groups[1].generalShader = VK_SHADER_UNUSED_NV;
	groups[1].anyHitShader = VK_SHADER_UNUSED_NV;
	groups[1].intersectionShader = VK_SHADER_UNUSED_NV;
	groups[1].type = VkRayTracingShaderGroupTypeNV::VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_NV;
	//miss
	groups[2] = {};
	groups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV;
	groups[2].pNext = nullptr;
	groups[2].generalShader = 2;
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
	//create bottom level acceleration strcuture
	//create cube shape to raytrace
	par_shapes_mesh* mesh = par_shapes_create_subdivided_sphere(3);
	par_shapes_scale(mesh, 2.0f, 2.0f, 2.0f);
	par_shapes_translate(mesh, -1.0f, -1.0f, -1.0f);
	par_shapes_unweld(mesh, true);
	par_shapes_compute_normals(mesh);

	eastl::vector<glm::vec3> vertices;
	eastl::vector<glm::vec3> normals;
	eastl::vector<uint32_t> indicies;
	for (int i = 0; i < mesh->npoints * 3; i += 3) {
		vertices.push_back(glm::vec3(mesh->points[i], mesh->points[i + 1], mesh->points[i + 2]));
		normals.push_back(glm::vec3(mesh->normals[i], mesh->normals[i + 1], mesh->normals[i + 2]));
	}
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		indicies.push_back(mesh->triangles[i + 0]);
		indicies.push_back(mesh->triangles[i + 1]);
		indicies.push_back(mesh->triangles[i + 2]);
	}
	par_shapes_free_mesh(mesh);

	m_InstanceBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, sizeof(VkGeometryInstanceNV) * 1000, nullptr);
	m_Indices = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(uint32_t) * indicies.size(), indicies.data());
	m_VertexPositions = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, sizeof(glm::vec3) * vertices.size(), vertices.data());

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
	vkCreateAccelerationStructureNV(device, &blasInfo, nullptr, &m_blas);
	//calc scratch buffer size
	VkAccelerationStructureMemoryRequirementsInfoNV asMemReqInfo = {};
	asMemReqInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	asMemReqInfo.pNext = nullptr;
	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
	asMemReqInfo.accelerationStructure = m_blas;
	VkMemoryRequirements2KHR memReqs = {};
	memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
	memReqs.pNext = nullptr;
	vkGetAccelerationStructureMemoryRequirementsNV(device, &asMemReqInfo, &memReqs);

	m_ScratchBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, 2 * 1024 * 1024, nullptr, memReqs.memoryRequirements.memoryTypeBits);

	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	vkGetAccelerationStructureMemoryRequirementsNV(device, &asMemReqInfo, &memReqs);
	m_BLASBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, memReqs.memoryRequirements.size, nullptr, memReqs.memoryRequirements.memoryTypeBits);

	VkBindAccelerationStructureMemoryInfoNV blasBindInfo = {};
	blasBindInfo.accelerationStructure = m_blas;
	blasBindInfo.memory = allocator.GetMemory(m_BLASBuffer.memory);
	blasBindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	blasBindInfo.pNext = nullptr;
	blasBindInfo.memoryOffset = allocator.GetMemoryOffset(m_BLASBuffer.memory);
	blasBindInfo.deviceIndexCount = 0;
	blasBindInfo.pDeviceIndices = nullptr;
	vkBindAccelerationStructureMemoryNV(device, 1, &blasBindInfo);

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

	asMemReqInfo.accelerationStructure = m_tlas;
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

	//transfer data
	glm::mat4x4 transform = glm::scale(glm::vec3(4, 4, 4)) * glm::translate(glm::vec3(0,1,0));
	transform = glm::transpose(transform);
	VkGeometryInstanceNV instanceData;
	vkGetAccelerationStructureHandleNV(device, m_blas, sizeof(uint64_t), &instanceData.accelerationStructureHandle);
	instanceData.flags = VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_NV;
	instanceData.instanceCustomIndex = 0;
	instanceData.hitGroupIndex = 0;
	instanceData.mask = 0xff;
	memcpy(instanceData.transform, &transform, sizeof(instanceData.transform));
	allocator.UpdateBuffer(m_InstanceBuffer, sizeof(VkGeometryInstanceNV), &instanceData);

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
	VkWriteDescriptorSet descWrites[3];
	VkDescriptorBufferInfo bufferDescs[1];
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
	vkUpdateDescriptorSets(device, _countof(descWrites), descWrites, 0, nullptr);
}

void RaytracingProgram::AddRenderQueueToTLAS(RenderQueue* queue, bool staticRenderQueue, DeviceAllocator& allocator) {
	uint32_t instanceCount = queue->GetInputs().size();
	eastl::vector<VkGeometryInstanceNV> instances(instanceCount);

	const std::map<ResourceHandle, ModelInstance>& models = queue->GetModels();
	uint32_t offset = 0;
	for (const auto& model : models) {
		for (uint32_t i = 0; i < model.second.Count; ++i) {
			VkGeometryInstanceNV& inst = instances[offset + i];
			//geoInstance.accelerationStructureHandle = blas; //TODO!!!
			inst.flags = 0;// TODO! add opaque flag here or in blas
			inst.hitGroupIndex = 0; //TODO! Add index to special geometry hit shader here
			inst.instanceCustomIndex = offset + i;
			inst.mask = 0xff;
			memcpy(inst.transform, &model.second.Inputs[i], sizeof(glm::mat3x4));
		}
		offset += model.second.Count;
	}

	if (staticRenderQueue) {
		m_StaticInstanceCount = instanceCount;
		allocator.UpdateBuffer(m_InstanceBuffer, instances.size() * sizeof(VkGeometryInstanceNV), instances.data());
		m_tlasInfo.instanceCount = m_StaticInstanceCount;
	} else {
		allocator.UpdateBuffer(m_InstanceBuffer, instances.size() * sizeof(VkGeometryInstanceNV), instances.data(), m_StaticInstanceCount * sizeof(VkGeometryInstanceNV));
		m_tlasInfo.instanceCount = m_StaticInstanceCount + instanceCount;
	}
}

void RaytracingProgram::BuildTLAS(CommandBuffer* cmdBuffer) {
	VkMemoryBarrier memoryBarrier = {};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
	memoryBarrier.pNext = nullptr;
	memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
	memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

	vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &m_tlasInfo, nullptr, 0, false, m_tlas, nullptr, m_ScratchBuffer.buffer, 0);
	vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
}

void RaytracingProgram::Update(CommandBuffer* cmdBuffer, DeviceAllocator* allocator, const CameraData* cd) {
	static bool first = true;
	if (first) {
		VkMemoryBarrier memoryBarrier = {};
		memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
		memoryBarrier.pNext = nullptr;
		memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
		memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;

		vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &m_blasInfo, nullptr, 0, false, m_blas, nullptr, m_ScratchBuffer.buffer, 0);
		vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

		m_tlasInfo.instanceCount = 1;
		vkCmdBuildAccelerationStructureNV(cmdBuffer->CmdBuffer(), &m_tlasInfo, m_InstanceBuffer.buffer, 0, false, m_tlas, nullptr, m_ScratchBuffer.buffer, 0);
		vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

		first = false;
	}

	PerFrameUniforms ub;
	ub.invViewProj = glm::inverse(cd->ProjView);
	ub.CamPos = glm::vec4(cd->Position, 1);
	ub.LightDir = glm::vec4(0, -1, 0, 0);

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
	allocator->DeAllocateBuffer(m_Indices);
	allocator->DeAllocateBuffer(m_VertexPositions);
	allocator->DeAllocateBuffer(m_InstanceBuffer);
	allocator->DeAllocateBuffer(m_UniformBuffer);
	allocator->DeAllocateBuffer(m_ScratchBuffer);
	allocator->DeAllocateBuffer(m_BLASBuffer);
	allocator->DeAllocateBuffer(m_TLASBuffer);
	allocator->DeAllocateBuffer(m_ShaderBindingTable);
	
}


#endif