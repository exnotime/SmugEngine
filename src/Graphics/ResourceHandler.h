#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <AssetLoader/AssetLoader.h>
#include "DeviceAllocator.h"
#include "VulkanContext.h"
#include "Texture.h"
#include "VkPipeline.h"
#include "FrameBuffer.h"

//use fixed size materials for now
//Albedo, Normal, Roughness, Metal
//If need to in the future add stuff like AO etc
//There is enough room to pack more into the material
#define MATERIAL_SIZE 4
#define MAX_MATERIALS 256

namespace smug {
enum VertexChannels {
	POSITION,
	NORMAL,
	TANGENT,
	TEXCOORD,
	NUM_VERTEX_CHANNELS
};

enum MaterialChannels {
	MC_ALBEDO,
	MC_ROUGHNESS,
	MC_NORMAL,
	MC_METAL,
	NUM_MATERIAL_CHANNELS
};

struct MaterialSet {
	uint32_t TextureCount;
	VkImageHandle* Textures;
	VkDescriptorSet DescSet;
};

struct Mesh {
	unsigned IndexCount;
	unsigned IndexOffset;
	MaterialSet Material;
};

struct Model {
	unsigned IndexCount;
	VkBufferHandle IndexBuffer;
	VkBufferHandle VertexBuffers[NUM_VERTEX_CHANNELS];
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
	void Init(VkDevice* device, const VkPhysicalDevice& physDev, MemoryBudget budget, DeviceAllocator& deviceAlloc, FrameBufferManager& fbManager);
	void ScheduleTransfer(CommandBuffer& cmdBuffer);
	void Clear();

	const Model& GetModel(ResourceHandle handle) const;
	const PipelineState& GetPipelineState(ResourceHandle handle) const;
	const VkBufferHandle& GetBuffer(ResourceHandle handle) const;

	void AllocateModel(const ModelInfo& model, ResourceHandle handle);
	void AllocateTexture(const TextureInfo& tex, ResourceHandle handle);
	void AllocateShader(const PipelineStateInfo& psInfo, ResourceHandle handle);
	void AllocateBuffer(uint64_t size, int usage, ResourceHandle handle);
	void ReAllocateModel(const ModelInfo& model, ResourceHandle handle);
	void UpdateBuffer(ResourceHandle buffer, size_t size, void* data);
	void DeAllocateModel(ResourceHandle handle);
	void DeAllocateTexture(ResourceHandle handle);
	void DeAllocateBuffer(ResourceHandle handle);
	ResourceAllocator& GetResourceAllocator() { return m_ResourceAllocator; }
  private:
	VkDevice* m_Device;
	DeviceAllocator* m_DeviceAllocator;
	FrameBufferManager* m_FrameBufferManager;
	ResourceAllocator m_ResourceAllocator;
	std::unordered_map<ResourceHandle, Model> m_Models;
	std::unordered_map<ResourceHandle, VkTexture> m_Textures;
	std::unordered_map<ResourceHandle, PipelineState> m_PipelineStates;
	std::unordered_map<ResourceHandle, VkBufferHandle> m_Buffers;

	VkTexture m_DefaultAlbedo;
	VkTexture m_DefaultNormal;
	VkTexture m_DefaultRoughness;
	VkTexture m_DefaultMetal;

	VkDescriptorPool m_DescPool;
	VkDescriptorSetLayout m_MaterialLayout;
	VkDescriptorSet m_DefaultMaterial;
};
}