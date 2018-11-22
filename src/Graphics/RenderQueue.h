#pragma once

///This class is the interface between the graphics engine and the game engine for fast path of models, lights and decals
///Fetch the renderqueue for the current frame from the graphics engine and dispatch renderqueue to the graphics engine to render everything
#include "GraphicsObjects.h"
#include "ResourceHandler.h"
#include "VulkanContext.h"
#include "DeviceAllocator.h"
#include <AssetLoader/Resources.h>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <Utility/Vector.h>

namespace smug {
struct ShaderInput {
	glm::mat4x4 Transform;
	glm::vec4 Color;
};

struct ModelInstance {
	std::vector<ShaderInput> Inputs;
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
	void AddModel(ResourceHandle handle, const glm::mat4& transform, const glm::vec4& tint);
	void ScheduleTransfer(DeviceAllocator& memory);
	void Destroy(ResourceHandler& resources);

	const std::vector<CameraData>& GetCameras() const {
		return m_Cameras;
	}
	const std::vector<ShaderInput>& GetInputs() const {
		return m_Inputs;
	}
	const std::map<ResourceHandle, ModelInstance>& GetModels() const {
		return m_Models;
	}

	const VkBufferHandle& GetUniformBuffer() const {
		return m_Resources->GetBuffer(m_Buffer);
	}
	void SetDescSet(vk::DescriptorSet set) {
		m_DescSet = set;
	}
	vk::DescriptorSet GetDescriptorSet() const {
		return m_DescSet;
	}

  private:
	std::vector<CameraData> m_Cameras;
	std::vector<ShaderInput> m_Inputs;
	std::map<ResourceHandle, ModelInstance> m_Models;
	vk::DescriptorSet m_DescSet;
	ResourceHandle m_Buffer;
	ResourceHandler* m_Resources;
};
}