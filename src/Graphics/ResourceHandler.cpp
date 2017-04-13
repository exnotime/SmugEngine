#include "ResourceHandler.h"
#include <AssetLoader/AssetLoader.h>

ResourceHandler::ResourceHandler() {

}
ResourceHandler::~ResourceHandler() {

}

ResourceHandle AllocModel(const ModelInfo& model, void* userData) {
	return ((ResourceHandler*)userData)->AllocateModel(model);
}

ResourceHandle AllocTexture(const TextureInfo& tex, void* userData) {
	return ((ResourceHandler*)userData)->AllocateTexture(tex);
}

void ResourceHandler::Init(const vk::Device& device, const vk::PhysicalDevice& physDev, MemoryBudget budget) {
	//split up geometry budget on different channels
	size_t vertexMemory = budget.GeometryBudget / 6;
	for (int c = 0; c < NUM_VERTEX_CHANNELS; ++c) {
		m_VertexMemory[c].Init(device, physDev, vertexMemory, vertexMemory);
		budget.GeometryBudget -= vertexMemory;
	}
	m_IndexMemory.Init(device, physDev, budget.GeometryBudget, budget.GeometryBudget);
	m_MaterialMemory.Init(device, physDev, budget.MaterialBudget, budget.MaterialBudget);


	ResourceAllocator allocator;
	allocator.AllocModel = AllocModel;
	allocator.AllocTexture = AllocTexture;
	allocator.ModelData = this;
	allocator.TextureData = this;
	g_AssetLoader->SetResourceAllocator(allocator);
}

ResourceHandle ResourceHandler::AllocateModel(const ModelInfo& model) {
	return 0;
}

ResourceHandle ResourceHandler::AllocateTexture(const TextureInfo& tex) {
	return 0;
}