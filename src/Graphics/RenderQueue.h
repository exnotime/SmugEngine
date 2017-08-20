#pragma once

///This class is the interface between the graphics engine and the game engine
///Fetch the renderqueue for the current frame from the graphics engine
#include "GraphicsObjects.h"
#include "VulkanContext.h"
#include "VkMemory.h"
#include <AssetLoader/Resources.h>
#include <vector>
#include <glm/glm.hpp>
struct ShaderInput {
	glm::mat4x4 Transform;
	glm::vec4 Color;
};

class GFX_DLL RenderQueue {
  public:
	RenderQueue();
	~RenderQueue();
	void Init(VkMemory& memory);
	void Clear();
	void AddCamera(const CameraData& cd);
	void AddModel(ResourceHandle handle, const glm::mat4& transform, const glm::vec4& tint);
	void ScheduleTransfer(VkMemory& memory);

	const std::vector<CameraData>& GetCameras() const { return m_Cameras; }
	const std::vector<ShaderInput>& GetInputs() const { return m_Inputs; }
	const std::vector<ResourceHandle>& GetModels() const {return m_Models;}

	const VkAlloc& GetUniformBuffer() const { return m_VkBuffer; }
	void SetDescSet(vk::DescriptorSet set) { m_DescSet = set; }
	vk::DescriptorSet GetDescriptorSet() { return m_DescSet; }


  private:
	std::vector<CameraData> m_Cameras;
	std::vector<ShaderInput> m_Inputs;
	std::vector<ResourceHandle> m_Models;
	vk::DescriptorSet m_DescSet;
	VkAlloc m_VkBuffer;
};