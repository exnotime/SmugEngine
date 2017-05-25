#include "ResourceHandler.h"
#include <AssetLoader/AssetLoader.h>
#define SAFE_DELETE(x) if(x) delete x

ResourceHandler::ResourceHandler() {

}
ResourceHandler::~ResourceHandler() {
	Clear();
}

ResourceHandle AllocModel(const ModelInfo& model, void* userData) {
	return ((ResourceHandler*)userData)->AllocateModel(model);
}

ResourceHandle AllocTexture(const TextureInfo& tex, void* userData) {
	return ((ResourceHandler*)userData)->AllocateTexture(tex);
}

void ResourceHandler::Init(vk::Device* device, const vk::PhysicalDevice& physDev, MemoryBudget budget) {
	//split up geometry budget on different channels
	size_t vertexMemory = budget.GeometryBudget / 6;
	for (int c = 0; c < NUM_VERTEX_CHANNELS; ++c) {
		m_VertexMemory[c].Init(*device, physDev, vertexMemory, vertexMemory);
		budget.GeometryBudget -= vertexMemory;
	}
	m_IndexMemory.Init(*device, physDev, budget.GeometryBudget, budget.GeometryBudget);
	m_MaterialMemory.Init(*device, physDev, budget.MaterialBudget, budget.MaterialBudget);


	ResourceAllocator allocator;
	allocator.AllocModel = AllocModel;
	allocator.AllocTexture = AllocTexture;
	allocator.ModelData = this;
	allocator.TextureData = this;
	g_AssetLoader.SetResourceAllocator(allocator);
	//descriptor pool
	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = MAX_MATERIALS;
	vk::DescriptorPoolSize poolSize;
	poolSize.descriptorCount = MAX_MATERIALS * MATERIAL_SIZE;
	poolSize.type = vk::DescriptorType::eCombinedImageSampler;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes = &poolSize;
	m_DescPool = device->createDescriptorPool(poolInfo);
	
	vk::DescriptorSetLayoutCreateInfo layoutInfo;
	layoutInfo.bindingCount = 1;
	vk::DescriptorSetLayoutBinding binding;
	binding.binding = 0;
	binding.descriptorCount = MATERIAL_SIZE;
	binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
	binding.stageFlags = vk::ShaderStageFlagBits::eAll;
	layoutInfo.pBindings = &binding;
	m_MaterialLayout = device->createDescriptorSetLayout(layoutInfo);

	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.descriptorPool = m_DescPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &m_MaterialLayout;
	m_DefaultMaterial = device->allocateDescriptorSets(allocInfo)[0];

	//default materials
	TextureInfo texInfo;
	texInfo.Width = 1;
	texInfo.Height = 1;
	texInfo.MipCount = 1;
	texInfo.Layers = 1;
	texInfo.Format = (uint32_t)vk::Format::eR8G8B8A8Unorm;
	texInfo.BPP = 32;
	texInfo.LinearSize = 4;
	texInfo.Data = malloc(4);
	memset(texInfo.Data, 0xFFFFFFFF, 4); // RGBA = 1,1,1,1
	m_DefaultAlbedo.Init(texInfo, m_MaterialMemory, *device);
	((uint8_t*)texInfo.Data)[0] = 128;
	((uint8_t*)texInfo.Data)[1] = 128;
	((uint8_t*)texInfo.Data)[2] = 255;
	((uint8_t*)texInfo.Data)[3] = 255;
	m_DefaultNormal.Init(texInfo, m_MaterialMemory, *device);
	memset(texInfo.Data, 0xFF00FFFF, 1);
	m_DefaultRoughness.Init(texInfo, m_MaterialMemory, *device);
	texInfo.LinearSize = 1;
	texInfo.Format = (uint32_t)vk::Format::eR8Unorm;
	memset(texInfo.Data, 0x0, 1);
	m_DefaultMetal.Init(texInfo, m_MaterialMemory, *device);
	free(texInfo.Data);

	m_Device = device;
}

const Model& ResourceHandler::GetModel(ResourceHandle handle) {
	return m_Models[(handle << 32) >> 32];
}

ResourceHandle ResourceHandler::AllocateModel(const ModelInfo& model) {
	Model internalModel;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<uint16_t> indices;
	internalModel.MeshCount = model.MeshCount;
	internalModel.Meshes = new Mesh[model.MeshCount];
	uint32_t indexCounter = 0;
	for (int m = 0; m < model.MeshCount; ++m) {
		//deinterleave the vertex data
		MeshInfo& mesh = model.Meshes[m];
		for (int v = 0; v < mesh.VertexCount; ++v) {
			positions.push_back(mesh.Vertices[v].Position);
			normals.push_back(mesh.Vertices[v].Normal);
			tangents.push_back(mesh.Vertices[v].Tangent);
			texcoords.push_back(mesh.Vertices[v].TexCoord);
		}
		//truncuate to 16 bit
		for (int i = 0; i < mesh.IndexCount; ++i) {
			indices.push_back(uint16_t(mesh.Indices[i]));
		}
		internalModel.Meshes[m].IndexCount = mesh.IndexCount;
		internalModel.Meshes[m].IndexOffset = indexCounter;
		indexCounter += mesh.IndexCount;

		//allocate material
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = m_DescPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &m_MaterialLayout;
		internalModel.Meshes[m].Material = m_Device->allocateDescriptorSets(allocInfo)[0];

		//create material
		vk::WriteDescriptorSet materialWrite;
		materialWrite.descriptorCount = MATERIAL_SIZE;
		materialWrite.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		materialWrite.dstArrayElement = 0;
		materialWrite.dstSet = internalModel.Meshes[m].Material;
		materialWrite.dstBinding = 0;
		vk::DescriptorImageInfo imageInfo[MATERIAL_SIZE];
		VkTexture albedo, normalTex, roughness, metal;
		//albedo
		if (model.Materials[mesh.Material].Albedo.Data) {
			albedo.Init(model.Materials[mesh.Material].Albedo, m_MaterialMemory, *m_Device);
			imageInfo[0] = albedo.GetDescriptorInfo();
		} else {
			imageInfo[0] = m_DefaultAlbedo.GetDescriptorInfo();
		}
		//normal
		if (model.Materials[mesh.Material].Normal.Data) {
			normalTex.Init(model.Materials[mesh.Material].Normal, m_MaterialMemory, *m_Device);
			imageInfo[1] = normalTex.GetDescriptorInfo();
		}
		else {
			imageInfo[1] = m_DefaultNormal.GetDescriptorInfo();
		}
		//roughness
		if (model.Materials[mesh.Material].Roughness.Data) {
			roughness.Init(model.Materials[mesh.Material].Roughness, m_MaterialMemory, *m_Device);
			imageInfo[2] = roughness.GetDescriptorInfo();
		}
		else {
			imageInfo[2] = m_DefaultRoughness.GetDescriptorInfo();
		}
		//metal
		if (model.Materials[mesh.Material].Metal.Data) {
			metal.Init(model.Materials[mesh.Material].Metal, m_MaterialMemory, *m_Device);
			imageInfo[3] = metal.GetDescriptorInfo();
		}
		else {
			imageInfo[3] = m_DefaultMetal.GetDescriptorInfo();
		}
		materialWrite.pImageInfo = imageInfo;
		m_Device->updateDescriptorSets(materialWrite, nullptr);
	}
	//allocate buffers
	vk::BufferUsageFlags flags = vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	internalModel.IndexBuffer = m_IndexMemory.AllocateBuffer(
		sizeof(uint16_t) * indices.size(), flags, indices.data());
	internalModel.IndexCount = indices.size();

	flags = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst;
	internalModel.VertexBuffers[POSITION] = m_VertexMemory[POSITION].AllocateBuffer(
			positions.size() * sizeof(glm::vec3), flags, positions.data());
	internalModel.VertexBuffers[NORMAL] = m_VertexMemory[NORMAL].AllocateBuffer(
		normals.size() * sizeof(glm::vec3), flags, normals.data());
	internalModel.VertexBuffers[TANGENT] = m_VertexMemory[TANGENT].AllocateBuffer(
		tangents.size() * sizeof(glm::vec3), flags, tangents.data());
	internalModel.VertexBuffers[TEXCOORD] = m_VertexMemory[TEXCOORD].AllocateBuffer(
		texcoords.size() * sizeof(glm::vec2), flags, texcoords.data());
	
	m_Models.push_back(internalModel);
	return m_Models.size() - 1;
}

ResourceHandle ResourceHandler::AllocateTexture(const TextureInfo& tex) {
	return 0;
}

void ResourceHandler::ScheduleTransfer(VulkanCommandBuffer& cmdBuffer) {
	m_VertexMemory[POSITION].ScheduleTransfers(cmdBuffer);
	m_VertexMemory[NORMAL].ScheduleTransfers(cmdBuffer);
	m_VertexMemory[TANGENT].ScheduleTransfers(cmdBuffer);
	m_VertexMemory[TEXCOORD].ScheduleTransfers(cmdBuffer);
	m_IndexMemory.ScheduleTransfers(cmdBuffer);
	m_MaterialMemory.ScheduleTransfers(cmdBuffer);
}

void ResourceHandler::Clear() {
	for (auto& m : m_Models) {
		SAFE_DELETE(m.Meshes);
	}
}