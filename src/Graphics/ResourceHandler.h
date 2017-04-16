#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <AssetLoader/AssetLoader.h>

#include "Memory.h"
#include "VulkanContext.h"
#include "Texture.h"

//use fixed size materials for now
//Albedo, Normal, Roughness, Metal
//If need to in the future add stuff like AO etc
//There is enough room to pack more into the material
#define MATERIAL_SIZE 4
#define MAX_MATERIALS 256


enum VertexChannels {
	POSITION,
	NORMAL,
	TANGENT,
	TEXCOORD,
	NUM_VERTEX_CHANNELS
};

struct Mesh {
	unsigned IndexCount;
	unsigned IndexOffset;
	vk::DescriptorSet Material;
};

struct Model {
	unsigned IndexCount;
	Buffer IndexBuffer;
	Buffer VertexBuffers[NUM_VERTEX_CHANNELS];
	unsigned MeshCount;
	Mesh* Meshes;
};

struct MemoryBudget {
	size_t GeometryBudget;
	size_t MaterialBudget;
};

class ResourceHandler {
public:
	ResourceHandler();
	~ResourceHandler();
	void Init(vk::Device* device, const vk::PhysicalDevice& physDev, MemoryBudget budget);
	void ScheduleTransfer(VulkanCommandBuffer& cmdBuffer);
	const Model& GetModel(ResourceHandle handle);

	ResourceHandle AllocateModel(const ModelInfo& model);
	ResourceHandle AllocateTexture(const TextureInfo& tex);
private:
	vk::Device* m_Device;
	std::vector<Model> m_Models;
	std::vector<VkTexture> m_Textures;

	VkTexture m_DefaultAlbedo;
	VkTexture m_DefaultNormal;
	VkTexture m_DefaultRoughness;
	VkTexture m_DefaultMetal;

	Memory m_VertexMemory[NUM_VERTEX_CHANNELS];
	Memory m_IndexMemory;
	Memory m_MaterialMemory;

	vk::DescriptorPool m_DescPool;
	vk::DescriptorSetLayout m_MaterialLayout;
	vk::DescriptorSet m_DefaultMaterial;
};