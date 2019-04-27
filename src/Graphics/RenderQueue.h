#pragma once

///This class is the interface between the graphics engine and the game engine for fast path of models, lights and decals
///Fetch the renderqueue for the current frame from the graphics engine and dispatch renderqueue to the graphics engine to render everything
#include "GraphicsObjects.h"
#include "ResourceHandler.h"
#include "VulkanContext.h"
#include "DeviceAllocator.h"
#include <AssetLoader/Resources.h>
#include <EASTL/vector.h>
#include <map>
#include <glm/glm.hpp>

#define MAX_INSTANCES 64 * 1024

namespace smug {
struct ShaderInput {
	glm::mat3x4 Transform;
	glm::vec4 Color;
};

struct ModelInstance {
	eastl::vector<ShaderInput> Inputs;
	uint32_t Layer;
	uint32_t Count;
	uint32_t Offset;
};

class GFX_DLL RenderQueue {
  public:
	RenderQueue();
	~RenderQueue();
	void Init(ResourceHandler& resources, int index);
	void Clear();
	void AddCamera(const CameraData& cd);
	void AddModel(ResourceHandle handle, const glm::mat3x4& transform, const glm::vec4& tint, uint32_t layer);
	void ScheduleTransfer(DeviceAllocator& memory);
	void Destroy(ResourceHandler& resources);

	const eastl::vector<CameraData>& GetCameras() const {
		return m_Cameras;
	}
	const eastl::vector<ShaderInput>& GetInputs() const {
		return m_Inputs;
	}
	const eastl::unordered_map<ResourceHandle, ModelInstance>& GetModels() const {
		return m_Models;
	}

	const VkBufferHandle& GetUniformBuffer() const {
		return m_Resources->GetBuffer(m_Buffer);
	}
	void SetDescSet(VkDescriptorSet set) {
		m_DescSet = set;
	}
	VkDescriptorSet GetDescriptorSet() const {
		return m_DescSet;
	}

  private:
	eastl::vector<CameraData> m_Cameras;
	eastl::vector<ShaderInput> m_Inputs;
	eastl::unordered_map<ResourceHandle, ModelInstance> m_Models;
	VkDescriptorSet m_DescSet;
	ResourceHandle m_Buffer;
	ResourceHandler* m_Resources;
};
}