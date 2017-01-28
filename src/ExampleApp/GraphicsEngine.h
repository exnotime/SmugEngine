#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VulkanContext.h"
#include <Tephra/TephraPipeline.h>

class GraphicsEngine {
public:
	GraphicsEngine();
	~GraphicsEngine();
	void Init(GLFWwindow* window);
	void Render();
	void Swap();
	void NextPipeline();
	void PrevPipeline();
private:
	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	CommandBuffer m_CmdBuffer;
	Tephra::Pipeline m_Pipelines[3];
	int m_CurrentPipeline;
	vk::RenderPass m_RenderPass;
	vk::Semaphore m_ImageAquiredSemaphore;
	vk::Semaphore m_RenderCompleteSemaphore;
	vk::Fence m_Fence[2];
	vk::Viewport m_Viewport;

	vk::DeviceMemory m_VertexMemory;
	vk::Buffer m_VertexBuffer;
	int m_MeshVertexCount;

	vk::Buffer m_UniformBuffer;
	vk::DeviceMemory m_UniformMemory;

	vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_DescriptorSet;

#ifdef _DEBUG
	VkDebugReportCallbackEXT m_DebugCallbacks;
#endif

};