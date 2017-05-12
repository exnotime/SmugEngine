#pragma once

///This class is the interface between the graphics engine and the game engine
///Fetch the renderqueue for the current frame from the graphics engine 
#include "GraphicsObjects.h"
#include "VulkanContext.h"
#include "VkMemory.h"
#include <AssetLoader/Resources.h>
#include <vector>
#include <glm/glm.hpp>
struct alignas(256) ShaderInput{
	glm::mat4 Transform;
	glm::vec4 Color;
};

class GFX_DLL RenderQueue {
public:
	RenderQueue();
	~RenderQueue();
	void Init(VkMemory& memory);
	void Clear();
	void AddCamera(const CameraData& cd);
	void AddModel(ResourceHandle handle, const ShaderInput& input);
	void ScheduleTransfer(VkMemory& memory);

	std::vector<CameraData>& GetCameras() { return m_Cameras; }
	std::vector<ShaderInput>& GetInputs() { return m_Inputs; }
	std::vector<ResourceHandle>& GetModels() { return m_Models; }
	VkAlloc& GetUniformBuffer() { return m_VkBuffer; }

	void SetDescSet(vk::DescriptorSet set) { m_DescSet = set; }
	vk::DescriptorSet GetDescriptorSet() { return m_DescSet; }

private:
	std::vector<CameraData> m_Cameras;
	std::vector<ShaderInput> m_Inputs;
	std::vector<ResourceHandle> m_Models;
	vk::DescriptorSet m_DescSet;
	VkAlloc m_VkBuffer;
};