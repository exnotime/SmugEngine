#pragma once
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <AssetLoader/AssetLoader.h>
#include "DeviceAllocator.h"
#include "VulkanContext.h"
#include "Texture.h"

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

	struct Mesh {
		unsigned IndexCount;
		unsigned IndexOffset;
		vk::DescriptorSet Material;
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
		void Init(vk::Device* device, const vk::PhysicalDevice& physDev, MemoryBudget budget, DeviceAllocator& deviceAlloc);
		void ScheduleTransfer(CommandBuffer& cmdBuffer);
		void Clear();

		const Model& GetModel(ResourceHandle handle) const;

		void AllocateModel(const ModelInfo& model, ResourceHandle handle);
		void AllocateTexture(const TextureInfo& tex, ResourceHandle handle);
	private:
		vk::Device* m_Device;
		DeviceAllocator* m_DeviceAllocator;

		std::unordered_map<ResourceHandle, Model> m_Models;
		std::unordered_map<ResourceHandle, VkTexture> m_Textures;

		VkTexture m_DefaultAlbedo;
		VkTexture m_DefaultNormal;
		VkTexture m_DefaultRoughness;
		VkTexture m_DefaultMetal;

		vk::DescriptorPool m_DescPool;
		vk::DescriptorSetLayout m_MaterialLayout;
		vk::DescriptorSet m_DefaultMaterial;
	};
}