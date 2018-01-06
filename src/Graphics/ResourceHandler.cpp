#include "ResourceHandler.h"
#include <AssetLoader/AssetLoader.h>
#include <Utility/Hash.h>

#define SAFE_DELETE(x) if(x) delete x

ResourceHandler::ResourceHandler() {

}
ResourceHandler::~ResourceHandler() {
	Clear();
}

ResourceHandle CreateHandle(const std::string name, RESOURCE_TYPE type) {
	ResourceHandle h = type;
	h = (h << RESOURCE_HASH_SHIFT);
	h |= HashString(name);
	return h;
}

void AllocateAssetCallback(const void* data, void* userData, const std::string& filename, const RESOURCE_TYPE type) {
	printf("Allocating asset %s\n", filename.c_str());
	if (type == RESOURCE_TYPE::RT_TEXTURE) {
		TextureInfo* info = (TextureInfo*)data;
		((ResourceHandler*)userData)->AllocateTexture(*info, CreateHandle(filename, type));
	}
	if (type == RESOURCE_TYPE::RT_MODEL) {
		ModelInfo* info = (ModelInfo*)data;
		((ResourceHandler*)userData)->AllocateModel(*info, CreateHandle(filename, type));
	}
}

void ResourceHandler::Init(vk::Device* device, const vk::PhysicalDevice& physDev, MemoryBudget budget, VkMemoryAllocator& deviceAlloc) {
	m_DeviceAllocator = &deviceAlloc;

	ResourceAllocator allocator;
	allocator.AllocResource = &AllocateAssetCallback;
	allocator.UserData = this;
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
	m_DefaultAlbedo.Init(texInfo, m_DeviceAllocator, *device);
	((uint8_t*)texInfo.Data)[0] = 128;
	((uint8_t*)texInfo.Data)[1] = 128;
	((uint8_t*)texInfo.Data)[2] = 255;
	((uint8_t*)texInfo.Data)[3] = 255;
	m_DefaultNormal.Init(texInfo, m_DeviceAllocator, *device);
	((uint8_t*)texInfo.Data)[0] = 128; //roughness
	((uint8_t*)texInfo.Data)[1] = 0; //metal
	((uint8_t*)texInfo.Data)[2] = 255; // ao
	((uint8_t*)texInfo.Data)[3] = 255; //unused
	m_DefaultRoughness.Init(texInfo, m_DeviceAllocator, *device);
	texInfo.LinearSize = 1;
	texInfo.Format = (uint32_t)vk::Format::eR8Unorm;
	memset(texInfo.Data, 0x0, 1);
	m_DefaultMetal.Init(texInfo, m_DeviceAllocator, *device);
	free(texInfo.Data);

	m_Device = device;
}

const Model& ResourceHandler::GetModel(ResourceHandle handle) {
	return m_Models[handle];
}

void ResourceHandler::AllocateModel(const ModelInfo& model, ResourceHandle handle) {
	Model internalModel;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> tangents;
	std::vector<glm::vec2> texcoords;
	std::vector<uint16_t> indices;
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

		//load materials

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
		if (model.Materials[mesh.Material].Albedo != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Albedo);
			if (t != m_Textures.end())
				imageInfo[0] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[0] = m_DefaultAlbedo.GetDescriptorInfo();
		}
		//normal
		if (model.Materials[mesh.Material].Normal != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Normal);
			if(t != m_Textures.end())
				imageInfo[1] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[1] = m_DefaultNormal.GetDescriptorInfo();
		}
		//roughness
		if (model.Materials[mesh.Material].Roughness != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Roughness);
			if (t != m_Textures.end())
				imageInfo[2] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[2] = m_DefaultRoughness.GetDescriptorInfo();
		}
		//metal
		if (model.Materials[mesh.Material].Metal != RESOURCE_INVALID) {
			auto& t = m_Textures.find(model.Materials[mesh.Material].Metal);
			if (t != m_Textures.end())
				imageInfo[3] = t->second.GetDescriptorInfo();
		} else {
			imageInfo[3] = m_DefaultMetal.GetDescriptorInfo();
		}
		materialWrite.pImageInfo = imageInfo;
		m_Device->updateDescriptorSets(materialWrite, nullptr);
	}
	//allocate buffers
	internalModel.IndexBuffer = m_DeviceAllocator->AllocateBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, sizeof(uint16_t) * indices.size(), indices.data());
	internalModel.IndexCount = (uint32_t)indices.size();

	vk::BufferCreateInfo vertBufferCreateInfo[NUM_VERTEX_CHANNELS];
	uint32_t sizes[] = { positions.size() * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3),
		tangents.size() * sizeof(glm::vec3), texcoords.size() * sizeof(glm::vec2) };
	void* datas[] = { positions.data(), normals.data(), tangents.data(), texcoords.data() };

	for (uint32_t i = 0; i < NUM_VERTEX_CHANNELS; ++i) {
		internalModel.VertexBuffers[i] = m_DeviceAllocator->AllocateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizes[i], datas[i]);
	}

	m_Models[handle] = internalModel;
}

void ResourceHandler::AllocateTexture(const TextureInfo& tex, ResourceHandle handle) {
	VkTexture texture;
	texture.Init(tex, m_DeviceAllocator, *m_Device);
	m_Textures[handle] = texture;
}

void ResourceHandler::ScheduleTransfer(VulkanCommandBuffer& cmdBuffer) {
	m_DeviceAllocator->ScheduleTransfers(cmdBuffer);
}

void ResourceHandler::Clear() {
	for (auto& m : m_Models) {
		SAFE_DELETE(m.second.Meshes);
	}
}