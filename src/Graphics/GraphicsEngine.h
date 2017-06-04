#pragma once
#include "GraphicsExport.h"
#include "VkPipeline.h"
#include "VulkanContext.h"
#include "VkMemory.h"
#include "Texture.h"
#include "SkyBox.h"
#include "Raymarch.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef USE_IMGUI
#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_glfw_vulkan.h>

struct ImguiInitData {
	void(*check_vk_result)(VkResult err);
	vk::PhysicalDevice Gpu;
	vk::Device Device;
	vk::RenderPass RenderPass;
	vk::DescriptorPool DescPool;
};
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
	void TransferToGPU();
	void Render();
	void Swap();
	RenderQueue* GetRenderQueue();

private:
	void CreateContext();
	void CreateSwapChain(VkSurfaceKHR surface);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBuffer m_vkCmdBuffer;
	VulkanCommandBuffer m_vkMarchCmdBuffer;
	VulkanCommandBuffer m_vkImguiCmdBuffer;
	VulkanQueue m_vkQueue;
	Tephra::VkPipeline m_Pipeline;
	int m_CurrentPipeline;

	vk::RenderPass m_RenderPass;
	vk::Semaphore m_ImageAquiredSemaphore;
	vk::Semaphore m_RenderCompleteSemaphore;
	vk::Semaphore m_RayMarchComplete;
	vk::Fence m_Fence[BUFFER_COUNT];
	vk::Viewport m_Viewport;

	VkAlloc m_PerFrameBuffer;
	SkyBox m_SkyBox;
	vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_PerFrameSet;
	//ibl
	VkTexture m_IBLTex;
	VkTexture m_SkyRad;
	VkTexture m_SkyIrr;
	vk::DescriptorSet m_IBLDescSet;
	//uniform and FBO memory
	VkMemory m_BufferMemory;
	VkMemory m_TextureMemory;

	bool m_MSAA;
	bool m_VSync;
	glm::vec2 m_ScreenSize;
	vk::PipelineMultisampleStateCreateInfo m_MSState;

	Raymarcher m_Raymarcher;
	VkDebugReportCallbackEXT m_DebugCallbacks;

	RenderQueue m_RenderQueues[BUFFER_COUNT];
	ResourceHandler m_Resources;

#ifdef USE_IMGUI
public:
	ImGui_ImplGlfwVulkan_Init_Data GetImguiInit();
	void CreateImguiFont(ImGuiContext* imguiCtx);
	vk::Semaphore m_ImguiComplete;
	vk::RenderPass m_ImguiRenderPass;
	ImGuiContext* m_ImguiCtx;
#endif

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
};
