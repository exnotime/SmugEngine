#pragma once
#include "GraphicsExport.h"
#include "VkPipeline.h"
#include "VulkanContext.h"
#include "Texture.h"
#include "SkyBox.h"
#include "RenderQueue.h"
#include "ResourceHandler.h"
#include "FrameBuffer.h"
#include "ToneMapProgram.h"

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
	glm::vec4 Material;
};

struct PerFrameStatistics {
	uint32_t ModelCount;
	uint32_t MeshCount;
	uint32_t TriangleCount;
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
	RenderQueue* GetStaticQueue();
	void PrintStats();

  private:
	void CreateContext();
	void CreateSwapChain(VkSurfaceKHR surface);
	void RenderModels(RenderQueue& rq, VulkanCommandBuffer& cmdBuffer);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBufferFactory m_CmdBufferFactory;

	VulkanQueue m_vkQueue;
	Tephra::VkPipeline m_Pipeline;
	int m_CurrentPipeline;

	vk::RenderPass m_RenderPass;
	vk::Semaphore m_ImageAquiredSemaphore;
	vk::Semaphore m_TransferComplete;
	vk::Semaphore m_RenderCompleteSemaphore;
	vk::Fence m_Fence[BUFFER_COUNT];
	vk::Viewport m_Viewport;
	vk::Rect2D m_Scissor;

	VkBufferHandle m_PerFrameBuffer;
	SkyBox m_SkyBox;
	vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_PerFrameSet;
	//ibl TODO: MOVE SOMEWHERE ELSE
	VkTexture m_IBLTex;
	VkTexture m_SkyRad;
	VkTexture m_SkyIrr;
	vk::DescriptorSet m_IBLDescSet;
	//uniform and FBO memory
	FrameBuffer m_FrameBuffer;

	bool m_VSync;
	glm::vec2 m_ScreenSize;
	vk::PipelineMultisampleStateCreateInfo m_MSState;

	ToneMapProgram m_ToneMapping;
	VkDebugReportCallbackEXT m_DebugCallbacks;

	RenderQueue m_RenderQueues[BUFFER_COUNT];
	RenderQueue m_StaticRenderQueue;
	ResourceHandler m_Resources;

	PerFrameStatistics m_Stats;
	//Device Allocator
	VkMemoryAllocator m_DeviceAllocator;


#ifdef USE_IMGUI
  public:
	ImGui_ImplGlfwVulkan_Init_Data GetImguiInit();
	void CreateImguiFont(ImGuiContext* imguiCtx);
  private:
	vk::Semaphore m_ImguiComplete;
	ImGuiContext* m_ImguiCtx;
#endif

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
};
