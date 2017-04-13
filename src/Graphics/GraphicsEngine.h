#pragma once
#include "GraphicsExport.h"

#include "TephraPipeline.h"
#include "VulkanContext.h"
#include "Memory.h"
#include "Texture.h"
#include "SkyBox.h"
#include "Raymarch.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif


struct PerFrameBuffer {
	glm::mat4 ViewProj;
	glm::vec4 CameraPos;
	glm::vec4 LightDir;
};

class GFX_DLL GraphicsEngine {
public:
	GraphicsEngine();
	~GraphicsEngine();
	void Init(glm::vec2 windowSize, bool vsync, HWND hWnd);
	void Render();
	void Swap();
	void NextPipeline();
	void PrevPipeline();
	RenderQueue* GetRenderQueue();
private:
	void CreateContext();
	void CreateSwapChain(VkSurfaceKHR surface);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBuffer m_vkCmdBuffer;
	VulkanCommandBuffer m_vkMarchCmdBuffer;
	VulkanQueue m_vkQueue;
	VkTexture m_Texture;
	Tephra::Pipeline m_Pipelines[3];
	int m_CurrentPipeline;

	vk::RenderPass m_RenderPass;
	vk::Semaphore m_ImageAquiredSemaphore;
	vk::Semaphore m_RenderCompleteSemaphore;
	vk::Semaphore m_RayMarchComplete;
	vk::Fence m_Fence[BUFFER_COUNT];
	vk::Viewport m_Viewport;

	Buffer m_VertexBuffer;
	int m_MeshVertexCount;

	Buffer m_UniformBuffer;
	SkyBox m_SkyBox;
	vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_DescriptorSet;

	Memory m_BufferMemory;
	Memory m_TextureMemory;

	bool m_MSAA;
	bool m_VSync;
	glm::vec2 m_ScreenSize;
	vk::PipelineMultisampleStateCreateInfo m_MSState;

	Raymarcher m_Raymarcher;
	VkDebugReportCallbackEXT m_DebugCallbacks;

	RenderQueue m_RenderQueues[BUFFER_COUNT];
	ResourceHandler m_Resources;

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
};
