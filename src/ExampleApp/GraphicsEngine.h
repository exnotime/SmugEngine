#pragma once
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "VulkanContext.h"
#include "Memory.h"
#include "Camera.h"
#include "Texture.h"
#include <Tephra/TephraPipeline.h>

struct PerFrameBuffer {
	glm::mat4 ViewProj;
	glm::vec4 CameraPos;
	glm::vec4 LightDir;
};

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
	void CreateContext();
	void CreateSwapChain(GLFWwindow* window, bool vsync);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBuffer m_vkCmdBuffer;
	VulkanQueue m_vkQueue;
	Texture m_Texture;
	Tephra::Pipeline m_Pipelines[3];
	int m_CurrentPipeline;

	vk::RenderPass m_RenderPass;
	vk::Semaphore m_ImageAquiredSemaphore;
	vk::Semaphore m_RenderCompleteSemaphore;
	vk::Fence m_Fence[BUFFER_COUNT];
	vk::Viewport m_Viewport;

	Buffer m_VertexBuffer;
	int m_MeshVertexCount;

	Buffer m_UniformBuffer;

	vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_DescriptorSet;

	Camera m_Camera;
	Memory m_BufferMemory;
	Memory m_TextureMemory;

	bool m_MSAA;
	bool m_VSync;
	vk::PipelineMultisampleStateCreateInfo m_MSState;

#ifdef _DEBUG
	VkDebugReportCallbackEXT m_DebugCallbacks;
#endif

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
};
