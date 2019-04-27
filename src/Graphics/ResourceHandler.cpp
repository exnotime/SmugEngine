#include "ResourceHandler.h"
#include <Utility/Hash.h>

#define SAFE_DELETE(x) if(x) delete x
using namespace smug;

ResourceHandler::ResourceHandler() {

}
ResourceHandler::~ResourceHandler() {
}

#pragma region AllocatorInterface
void AllocateAssetCallback(const void* data, void* userData, const eastl::string& filename, const RESOURCE_TYPE type) {
	printf("Allocating asset %s\n", filename.c_str());
	if (type == RESOURCE_TYPE::RT_TEXTURE) {
		TextureInfo* info = (TextureInfo*)data;
		((ResourceHandler*)userData)->AllocateTexture(*info, CreateHandle(HashString(filename), type));
	}
	if (type == RESOURCE_TYPE::RT_MODEL) {
		ModelInfo* info = (ModelInfo*)data;
		((ResourceHandler*)userData)->AllocateModel(*info, CreateHandle(HashString(filename), type));
	}
	if (type == RESOURCE_TYPE::RT_SHADER) {
		PipelineStateInfo* info = (PipelineStateInfo*)data;
		((ResourceHandler*)userData)->AllocateShader(*info, CreateHandle(HashString(filename), type));
	}
}

void DeAllocateAssetCallback(ResourceHandle handle, void* userData) {
	RESOURCE_TYPE type = GetType(handle);
	if (type == RESOURCE_TYPE::RT_TEXTURE) {
		((ResourceHandler*)userData)->DeAllocateTexture(handle);
	}
	if (type == RESOURCE_TYPE::RT_MODEL) {
		((ResourceHandler*)userData)->DeAllocateModel(handle);
	}
}

void ReAllocateAssetCallback(const void* data, void* userData, ResourceHandle handle) {
	RESOURCE_TYPE type = GetType(handle);

	if (type == RESOURCE_TYPE::RT_TEXTURE) {
	}
	if (type == RESOURCE_TYPE::RT_MODEL) {
		ModelInfo* info = (ModelInfo*)data;
		((ResourceHandler*)userData)->ReAllocateModel(*info,handle);
	}
}
#pragma endregion

void ResourceHandler::Init(VkDevice* device, const VkPhysicalDevice& physDev, MemoryBudget budget, DeviceAllocator& deviceAlloc, FrameBufferManager& fbManager) {
	m_DeviceAllocator = &deviceAlloc;

	m_ResourceAllocator.AllocResource = &AllocateAssetCallback;
	m_ResourceAllocator.DeAllocResource = &DeAllocateAssetCallback;
	m_ResourceAllocator.ReAllocResource = &ReAllocateAssetCallback;
	m_ResourceAllocator.UserData = this;
	g_AssetLoader.SetResourceAllocator(m_ResourceAllocator);

	//descriptor pool
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.maxSets = MAX_MATERIALS;
	VkDescriptorPoolSize poolSize;
	poolSize.descriptorCount = MAX_MATERIALS * MATERIAL_SIZE;
	poolSize.type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	vkCreateDescriptorPool(*device, &poolInfo, nullptr, &m_DescPool);

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = 1;
	VkDescriptorSetLayoutBinding binding = {};
	binding.binding = 0;
	binding.descriptorCount = MATERIAL_SIZE;
	binding.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.stageFlags = VkShaderStageFlagBits::VK_SHADER_STAGE_ALL;
	layoutInfo.pBindings = &binding;
	vkCreateDescriptorSetLayout(*device, &layoutInfo, nullptr, &m_MaterialLayout);

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.descriptorPool = m_DescPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_MaterialLayout;
	vkAllocateDescriptorSets(*device, &allocInfo, &m_DefaultMaterial);

	//default materials
	TextureInfo texInfo;
	texInfo.Width = 1;
	texInfo.Height = 1;
	texInfo.MipCount = 1;
	texInfo.Layers = 1;
	texInfo.Format = (uint32_t)VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
	texInfo.BPP = 32;
	texInfo.LinearSize = 4;
	texInfo.Data = malloc(4);
	memset(texInfo.Data, 0xFFFFFFFF, 4); // RGBA = 1,1,1,1
	m_DefaultAlbedo.Init(texInfo, m_DeviceAllocator, *device);
	((uint8_t*)texInfo.Data)[0] = 128;
	((uint8_t*)texInfo.Data)[1] = 128;
	((uint8_t*)texInfo.Data)[2] = 255;
	((uint8_t*)texInfo.Data)[3] = 255;
	m_DefaultNormal.Init(texInfo, m_DeviceAllocator, *device);
	((uint8_t*)texInfo.Data)[0] = 230; //roughness
	((uint8_t*)texInfo.Data)[1] = 0; //metal
	((uint8_t*)texInfo.Data)[2] = 255; // ao
	((uint8_t*)texInfo.Data)[3] = 255; //unused
	m_DefaultRoughness.Init(texInfo, m_DeviceAllocator, *device);
	texInfo.LinearSize = 1;
	texInfo.Format = (uint32_t)VkFormat::VK_FORMAT_R8_UNORM;
	memset(texInfo.Data, 0x0, 1);
	m_DefaultMetal.Init(texInfo, m_DeviceAllocator, *device);
	free(texInfo.Data);

	m_FrameBufferManager = &fbManager;
	m_Device = device;
}

const Model& ResourceHandler::GetModel(ResourceHandle handle) const {
	return m_Models.find(handle)->second;
}

const PipelineState& ResourceHandler::GetPipelineState(ResourceHandle handle) const {
	return m_PipelineStates.find(handle)->second;
}

const VkBufferHandle& ResourceHandler::GetBuffer(ResourceHandle handle) const {
	return m_Buffers.find(handle)->second;
}

void ResourceHandler::AllocateModel(const ModelInfo& model, ResourceHandle handle) {
	Model internalModel;
	eastl::vector<glm::vec3> positions;
	eastl::vector<glm::vec3> normals;
	eastl::vector<glm::vec3> tangents;
	eastl::vector<glm::vec2> texcoords;
	eastl::vector<uint32_t> indices;
	internalModel.MeshCount = model.MeshCount;
	internalModel.Meshes = new Mesh[model.MeshCount];
	uint32_t indexCounter = 0;
	for (uint32_t m = 0; m < model.MeshCount; ++m) {
		//deinterleave the vertex data
		MeshInfo& mesh = model.Meshes[m];
		for (uint32_t v = 0; v < mesh.VertexCount; ++v) {
			positions.push_back(mesh.Vertices[v].Position);
			normals.push_back(mesh.Vertices[v].Normal);
			tangents.push_back(mesh.Vertices[v].Tangent);
			texcoords.push_back(mesh.Vertices[v].TexCoord);
		}
		//truncuate to 16 bit
		for (uint32_t i = 0; i < mesh.IndexCount; ++i) {
			indices.push_back(mesh.Indices[i]);
		}
		internalModel.Meshes[m].IndexCount = mesh.IndexCount;
		internalModel.Meshes[m].IndexOffset = indexCounter;
		indexCounter += mesh.IndexCount;

		//allocate material
		VkDescriptorSetAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = m_DescPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_MaterialLayout;
		vkAllocateDescriptorSets(*m_Device, &allocInfo, &internalModel.Meshes[m].Material.DescSet);
		internalModel.Meshes[m].Material.TextureCount = 0; //TEMP!
		//create material
		VkWriteDescriptorSet materialWrite;
		materialWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		materialWrite.pNext = nullptr;
		materialWrite.descriptorCount = MATERIAL_SIZE;
		materialWrite.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		materialWrite.dstArrayElement = 0;
		materialWrite.dstSet = internalModel.Meshes[m].Material.DescSet;
		materialWrite.dstBinding = 0;
		VkDescriptorImageInfo imageInfo[MATERIAL_SIZE];
		//albedo
		if (model.Materials && model.Materials[mesh.Material].Albedo != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Albedo);
			if (t != m_Textures.end())
				imageInfo[0] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[0] = m_DefaultAlbedo.GetDescriptorInfo();
		}
		//normal
		if (model.Materials && model.Materials[mesh.Material].Normal != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Normal);
			if(t != m_Textures.end())
				imageInfo[1] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[1] = m_DefaultNormal.GetDescriptorInfo();
		}
		//roughness
		if (model.Materials && model.Materials[mesh.Material].Roughness != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Roughness);
			if (t != m_Textures.end())
				imageInfo[2] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[2] = m_DefaultRoughness.GetDescriptorInfo();
		}
		//metal
		if (model.Materials && model.Materials[mesh.Material].Metal != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Metal);
			if (t != m_Textures.end())
				imageInfo[3] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[3] = m_DefaultMetal.GetDescriptorInfo();
		}
		materialWrite.pImageInfo = imageInfo;
		vkUpdateDescriptorSets(*m_Device, 1, &materialWrite, 0, nullptr);
	}
	//allocate buffers
	internalModel.IndexBuffer = m_DeviceAllocator->AllocateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint32_t) * indices.size(), indices.data());
	internalModel.IndexCount = (uint32_t)indices.size();

	//VkBufferCreateInfo vertBufferCreateInfo[NUM_VERTEX_CHANNELS];
	uint32_t sizes[] = { (uint32_t)positions.size() * sizeof(glm::vec3), (uint32_t)normals.size() * sizeof(glm::vec3),
	                     (uint32_t)tangents.size() * sizeof(glm::vec3), (uint32_t)texcoords.size() * sizeof(glm::vec2)
	                   };
	void* datas[] = { positions.data(), normals.data(), tangents.data(), texcoords.data() };

	for (uint32_t i = 0; i < NUM_VERTEX_CHANNELS; ++i) {
		internalModel.VertexBuffers[i] = m_DeviceAllocator->AllocateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizes[i], datas[i]);
	}

#ifdef RTX_ON
	//allocate blas
	VkGeometryNV blasGeometry = {};
	blasGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_NV;
	blasGeometry.geometry.triangles.indexCount = internalModel.IndexCount;
	blasGeometry.geometry.triangles.indexData = internalModel.IndexBuffer.buffer;
	blasGeometry.geometry.triangles.indexOffset = 0;
	blasGeometry.geometry.triangles.indexType = VkIndexType::VK_INDEX_TYPE_UINT32;
	blasGeometry.geometry.triangles.vertexCount = positions.size();
	blasGeometry.geometry.triangles.vertexData = internalModel.VertexBuffers[POSITION].buffer;
	blasGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
	blasGeometry.geometry.triangles.vertexOffset = 0;
	blasGeometry.geometry.triangles.vertexStride = sizeof(glm::vec3);
	blasGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV;
	blasGeometry.geometry.triangles.pNext = nullptr;
	blasGeometry.geometry.triangles.transformData = nullptr;
	blasGeometry.geometry.triangles.transformOffset = 0;
	blasGeometry.geometry.aabbs.sType = VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV;
	blasGeometry.geometry.aabbs.pNext = nullptr;
	blasGeometry.geometry.aabbs.aabbData = nullptr;
	blasGeometry.geometry.aabbs.numAABBs = 0;
	blasGeometry.geometry.aabbs.offset = 0;
	blasGeometry.geometry.aabbs.stride = 0;
	blasGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_NV;
	blasGeometry.sType = VK_STRUCTURE_TYPE_GEOMETRY_NV;
	blasGeometry.pNext = nullptr;

	VkAccelerationStructureCreateInfoNV blasCreateInfo = {};
	blasCreateInfo.info.instanceCount = 0;
	blasCreateInfo.info.geometryCount = 1;
	blasCreateInfo.info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
	blasCreateInfo.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
	blasCreateInfo.info.pNext = nullptr;
	blasCreateInfo.info.pGeometries = &blasGeometry;
	blasCreateInfo.info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
	blasCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
	blasCreateInfo.pNext = nullptr;
	//create blas
	vkCreateAccelerationStructureNV(*m_Device, &blasCreateInfo, nullptr, &internalModel.Blas.Handle);
	//calc scratch buffer size
	VkAccelerationStructureMemoryRequirementsInfoNV asMemReqInfo = {};
	asMemReqInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
	asMemReqInfo.pNext = nullptr;
	asMemReqInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
	asMemReqInfo.accelerationStructure = internalModel.Blas.Handle;
	VkMemoryRequirements2KHR memReqs = {};
	memReqs.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2_KHR;
	memReqs.pNext = nullptr;
	vkGetAccelerationStructureMemoryRequirementsNV(*m_Device, &asMemReqInfo, &memReqs);
	internalModel.Blas.Size = memReqs.memoryRequirements.size;
	internalModel.Blas.Memory = m_DeviceAllocator->AllocateBuffer(VK_BUFFER_USAGE_RAY_TRACING_BIT_NV, internalModel.Blas.Size, nullptr, memReqs.memoryRequirements.memoryTypeBits);

	VkBindAccelerationStructureMemoryInfoNV blasBindInfo = {};
	blasBindInfo.accelerationStructure = internalModel.Blas.Handle;
	blasBindInfo.memory = m_DeviceAllocator->GetMemory(internalModel.Blas.Memory.memory);
	blasBindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
	blasBindInfo.pNext = nullptr;
	blasBindInfo.memoryOffset = m_DeviceAllocator->GetMemoryOffset(internalModel.Blas.Memory.memory);
	blasBindInfo.deviceIndexCount = 0;
	blasBindInfo.pDeviceIndices = nullptr;
	vkBindAccelerationStructureMemoryNV(*m_Device, 1, &blasBindInfo);
#endif

	m_Models[handle] = internalModel;
}

void ResourceHandler::AllocateTexture(const TextureInfo& tex, ResourceHandle handle) {
	VkTexture texture;
	texture.Init(tex, m_DeviceAllocator, *m_Device);
	m_Textures[handle] = texture;
}

void ResourceHandler::AllocateShader(const PipelineStateInfo& psInfo, ResourceHandle handle) {
	PipelineState ps;
	//create renderpass if needed
	VkRenderPass rp = nullptr;
	//if (psInfo.RenderTargetCount > 0) {
	//	SubPass sp;
	//	for (uint32_t i = 0; i < psInfo.RenderTargetCount; ++i) {
	//		sp.RenderTargets.push_back(psInfo.RenderTargets[i]);
	//	}
	//	sp.DepthStencilAttachment = psInfo.DepthStencil;
	//	rp = m_FrameBufferManager->CreateRenderPass(0xDEAD1331,{ sp });
	//}
	ps.LoadPipelineFromInfo(*m_Device, psInfo, rp);
	m_PipelineStates[handle] = ps;
}

void ResourceHandler::AllocateBuffer(uint64_t size, int usage, ResourceHandle handle) {
	m_Buffers[handle] = m_DeviceAllocator->AllocateBuffer(usage, size, nullptr);
}

void ResourceHandler::ReAllocateModel(const ModelInfo& model, ResourceHandle handle) {
	DeAllocateModel(handle);
	AllocateModel(model, handle);
}

void ResourceHandler::UpdateBuffer(ResourceHandle buffer, size_t size, void* data) {
	m_DeviceAllocator->UpdateBuffer(m_Buffers[buffer], size, data);
}

void ResourceHandler::DeAllocateModel(ResourceHandle handle) {
	auto& model = m_Models.find(handle);
	if (model != m_Models.end()) {
		auto& m = model->second;
		//deallocate geometry
		m_DeviceAllocator->DeAllocateBuffer(m.IndexBuffer);
		for (uint32_t i = 0; i < NUM_VERTEX_CHANNELS; ++i) {
			m_DeviceAllocator->DeAllocateBuffer(m.VertexBuffers[i]);
		}
		//deallocate materials
		for (uint32_t i = 0; i < m.MeshCount; ++i) {
			auto& mesh = m.Meshes[i];
			if (mesh.Material.Textures) {
				for (uint32_t t = 0; t < mesh.Material.TextureCount; ++t) {
					m_DeviceAllocator->DeAllocateImage(mesh.Material.Textures[t]);
				}
			}
		}
		delete[] m.Meshes;
	}
	m_Models.erase(handle);
}

void ResourceHandler::DeAllocateTexture(ResourceHandle handle) {
	auto& tex = m_Textures.find(handle);
	if (tex != m_Textures.end()){
		m_DeviceAllocator->DeAllocateImage(tex->second.GetImageHandle());
		m_Textures.erase(handle);
	}
}

void ResourceHandler::DeAllocateBuffer(ResourceHandle handle) {
	auto& buffer = m_Buffers.find(handle);
	if (buffer != m_Buffers.end()) {
		m_DeviceAllocator->DeAllocateBuffer(m_Buffers[handle]);
		m_Buffers.erase(handle);
	}
}

void ResourceHandler::ScheduleTransfer(CommandBuffer& cmdBuffer) {
	m_DeviceAllocator->ScheduleTransfers(&cmdBuffer);
}

void ResourceHandler::Clear() {
	for (auto& texture : m_Textures) {
		m_DeviceAllocator->DeAllocateImage(texture.second.GetImageHandle());
	}

	for (auto& m : m_Models) {
		m_DeviceAllocator->DeAllocateBuffer(m.second.IndexBuffer);
		for (int i = 0; i < 4; ++i) {
			m_DeviceAllocator->DeAllocateBuffer(m.second.VertexBuffers[i]);
		}
		SAFE_DELETE(m.second.Meshes);
	}
	m_DeviceAllocator->DeAllocateImage(m_DefaultAlbedo.GetImageHandle());
	m_DeviceAllocator->DeAllocateImage(m_DefaultMetal.GetImageHandle());
	m_DeviceAllocator->DeAllocateImage(m_DefaultNormal.GetImageHandle());
	m_DeviceAllocator->DeAllocateImage(m_DefaultRoughness.GetImageHandle());
}