#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <AssetLoader/AssetLoader.h>

#include "Memory.h"
#include "VulkanContext.h"
//use fixed size materials for now
//Albedo, Normal, Roughness, Metal
//If need to in the future add stuff like AO etc
//There is enough room to pack more into the material
#define MATERIAL_SIZE 4 

struct Mesh {
	glm::vec3 Min;
	glm::vec3 Max;
	unsigned IndexCount;
	unsigned IndexOffset;
	unsigned Material;
};

struct Model {
	glm::vec3 Min;
	glm::vec3 Max;
	unsigned IndexCount;
	unsigned IndexOffset;
	unsigned MaterialOffset;
	Mesh* Meshes;
	unsigned MeshCount;
};

struct Texture {
	unsigned DescriptorOffset;
};

struct MemoryBudget {
	size_t GeometryBudget;
	size_t MaterialBudget;
};

enum VertexChannels {
	POSITION,
	NORMAL,
	TANGENT,
	TEXCOORD,
	NUM_VERTEX_CHANNELS
};

class ResourceHandler {
public:
	ResourceHandler();
	~ResourceHandler();
	void Init(const vk::Device& device, const vk::PhysicalDevice& physDev, MemoryBudget budget);
	const Model& GetModel(uint64_t handle);

	ResourceHandle AllocateModel(const ModelInfo& model);
	ResourceHandle AllocateTexture(const TextureInfo& tex);
private:
	std::vector<Model> m_Models;
	std::vector<Texture> m_Textures;

	Memory m_VertexMemory[NUM_VERTEX_CHANNELS];
	Memory m_IndexMemory;
	Memory m_MaterialMemory;
	
};