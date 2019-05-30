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
#include "VulkanProfiler.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef USE_IMGUI
#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_glfw_vulkan.h>
#endif

namespace smug {

	class RaytracingProgram;
	class LightingProgram;

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
	void InitPipeline();
	void DeInit();
	void TransferToGPU();
	void Render();
	void Swap();
	RenderQueue* GetRenderQueue();
	RenderQueue* GetStaticQueue();
	ResourceAllocator& GetResourceAllocator() { return m_Resources->GetResourceAllocator(); }
	FrameBufferManager& GetFrameBufferManager() { return m_FrameBuffer; }
	ResourceHandler& GetResourceHandler() { return *m_Resources; }
	void PrintStats();

  private:
	void CreateContext();
	void CreateSwapChain(VkSurfaceKHR surface);
	void RenderModels(RenderQueue& rq, CommandBuffer& cmdBuffer, VkViewport vp);

	VulkanContext m_VKContext;
	VulkanSwapChain m_VKSwapChain;
	VulkanCommandBufferFactory* m_CmdBufferFactory;

	DeviceQueue m_vkQueue;
	PipelineState m_Pipeline;

	VkRenderPass m_RenderPass;

	VkSemaphore m_ImageAquiredSemaphore;
	VkSemaphore m_TransferComplete;
	VkSemaphore m_RenderCompleteSemaphore;
	VkFence m_Fence[BUFFER_COUNT];
	VkViewport m_Viewport;
	VkRect2D m_Scissor;

	VkBufferHandle m_PerFrameBuffer;
	SkyBox m_SkyBox;
	VkDescriptorSet m_PerFrameSet;
	//ibl TODO: MOVE SOMEWHERE ELSE
	VkTexture m_IBLTex;
	VkTexture m_SkyRad;
	VkTexture m_SkyIrr;
	VkDescriptorSet m_IBLDescSet;

	FrameBufferManager m_FrameBuffer;
	
	LightingProgram* m_LightProgram;
	bool m_VSync;
	glm::vec2 m_ScreenSize;
	VkPipelineMultisampleStateCreateInfo m_MSState;
	ShadowMapProgram* m_ShadowProgram;
	ToneMapProgram* m_ToneMapping;
	RaytracingProgram* m_RaytracingProgram;
	VkDebugReportCallbackEXT m_DebugCallbacks;
	RenderQueue* m_RenderQueues;
	RenderQueue* m_StaticRenderQueue;
	ResourceHandler* m_Resources;
	PerFrameStatistics m_Stats;
	DeviceAllocator m_DeviceAllocator;
	VulkanProfiler m_Profiler;

	uint32_t m_ColorPassHash = 0;
	uint32_t m_AlbedoTargetHash = 0;
	uint32_t m_NormalsTargetHash = 0;
	uint32_t m_MaterialTargetHash = 0;
	uint32_t m_DepthTargetHash = 0;
	uint32_t m_ShadowTargetHash = 0;
	uint32_t m_HDRTargetHash = 0;

#ifdef USE_IMGUI
  public:
	ImGui_ImplGlfwVulkan_Init_Data* GetImguiInit();
	void CreateImguiFont(ImGuiContext* imguiCtx);
  private:
	VkSemaphore m_ImguiComplete;
#endif

#define VK_DEVICE m_VKContext.Device
#define VK_PHYS_DEVICE m_VKContext.PhysicalDevice
#define VK_FRAME_INDEX m_VKContext.FrameIndex
#define VK_DESC_POOL m_VKContext.DescriptorPool
};
}
