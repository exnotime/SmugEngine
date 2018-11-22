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
#include "ShadowMapProgram.h"
#include "PipelineStateEditor.h"
#include "RenderPipeline.h"
#include "RaytracingProgram.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef USE_IMGUI

#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_glfw_vulkan.h>
#endif

namespace smug {

struct PerFrameBuffer {
	glm::mat4 ViewProj;
	glm::mat4 View;
	glm::vec4 CameraPos;
	glm::vec4 LightDir;
	glm::vec4 Material;
	glm::mat4 LightViewProj[4];
	glm::vec4 NearFarPadding;
	glm::vec4 ShadowSplits;
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
	ResourceAllocator& GetResourceAllocator() { return m_Resources.GetResourceAllocator(); }
	FrameBufferManager& GetFrameBufferManager() { return m_FrameBuffer; }
	RenderPipeline& GetRenderPipeline() { return m_RenderPipeline; }
	ResourceHandler& GetResourceHandler() { return m_Resources; }
	void PrintStats();

  private:
	void CreateContext();
	void CreateSwapChain(VkSurfaceKHR surface);
	void RenderModels(RenderQueue& rq, CommandBuffer& cmdBuffer);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBufferFactory m_CmdBufferFactory;

	DeviceQueue m_vkQueue;
	PipelineState m_Pipeline;
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
	//vk::DescriptorPool m_DescriptorPool;
	vk::DescriptorSet m_PerFrameSet;
	//ibl TODO: MOVE SOMEWHERE ELSE
	VkTexture m_IBLTex;
	VkTexture m_SkyRad;
	VkTexture m_SkyIrr;
	vk::DescriptorSet m_IBLDescSet;

	FrameBufferManager m_FrameBuffer;

	bool m_VSync;
	glm::vec2 m_ScreenSize;
	vk::PipelineMultisampleStateCreateInfo m_MSState;
	ShadowMapProgram m_ShadowProgram;
	ToneMapProgram m_ToneMapping;

	VkDebugReportCallbackEXT m_DebugCallbacks;
	RenderQueue m_RenderQueues[BUFFER_COUNT];
	RenderQueue m_StaticRenderQueue;
	ResourceHandler m_Resources;
	PerFrameStatistics m_Stats;
	DeviceAllocator m_DeviceAllocator;
	RenderPipeline m_RenderPipeline;
	RaytracingProgram m_RaytracingProgram;

#ifdef USE_IMGUI
  public:
	ImGui_ImplGlfwVulkan_Init_Data* GetImguiInit();
	void CreateImguiFont(ImGuiContext* imguiCtx);
  private:
	vk::Semaphore m_ImguiComplete;
	ImGuiContext* m_ImguiCtx;

	PipelineStateEditor pipelineEditor;
#endif

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
#define VK_DESC_POOL m_VKContext.DescriptorPool
};
}
