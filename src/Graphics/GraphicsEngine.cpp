#include "GraphicsEngine.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <vulkan/vulkan.h>
#include "Vertex.h"
#include "Core/Timer.h"
#include "Core/Input.h"
#define USE_DEBUG_LAYER 0
using namespace smug;

GraphicsEngine::GraphicsEngine() {

}

GraphicsEngine::~GraphicsEngine() {
#ifdef USE_IMGUI
	ImGui_ImplGlfwVulkan_Shutdown();
#endif
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT                  messageType,
    const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
    void*                                            pUserData) {

	if (pCallbackData->messageIdNumber == 3 || pCallbackData->messageIdNumber == 15 || pCallbackData->messageIdNumber == 1338) {
		return VK_FALSE;
	}

#ifdef _WIN32
	wchar_t msg[1024];
	mbstowcs(msg, pCallbackData->pMessage, 1024);
	OutputDebugString(msg);
	OutputDebugString(L"\n");
#endif

	printf("%s\n", pCallbackData->pMessage);

	return VK_FALSE;
}

void GraphicsEngine::CreateContext() {

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "SmugEngine";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "SmugEngine";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_1;

	Vector<const char*> layers;
	Vector<const char*> extensions;

#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	extensions.push_back("VK_EXT_debug_utils");
	//layers.push_back("VK_LAYER_RENDERDOC_Capture");
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
	messengerCreateInfo.flags = 0;
	messengerCreateInfo.messageSeverity =	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT	|
	                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	messengerCreateInfo.messageType =		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
	                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
	                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	messengerCreateInfo.pNext = nullptr;
	messengerCreateInfo.pUserData = nullptr;
	messengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	messengerCreateInfo.pfnUserCallback = &VulkanDebugCallback;
#endif
	extensions.push_back("VK_KHR_surface");
	extensions.push_back("VK_KHR_get_physical_device_properties2");
#ifdef _WIN32
	extensions.push_back("VK_KHR_win32_surface");
#endif

	vk::InstanceCreateInfo instInfo;
	instInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instInfo.pApplicationInfo = &appInfo;
	instInfo.ppEnabledExtensionNames = &extensions[0];
	instInfo.enabledLayerCount = (uint32_t)layers.size();
	instInfo.ppEnabledLayerNames = &layers[0];
#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	//push into instance create info to detect problem during initialization
	instInfo.pNext = &messengerCreateInfo;
#endif
	m_VKContext.Instance = vk::createInstance(instInfo);


#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	VkDebugUtilsMessengerEXT messenger;
	PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)m_VKContext.Instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
	CreateDebugUtilsMessengerEXT(m_VKContext.Instance, &messengerCreateInfo, nullptr, &messenger);
#endif

	auto gpus = m_VKContext.Instance.enumeratePhysicalDevices();
	if (gpus.size() == 0) {
		return;
	}
	//grab first available GPU
	for (auto& gpu : gpus) {
		if (gpu.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
		        gpu.getProperties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
			VK_PHYS_DEVICE = gpu;
			break;
		}
	}

	m_vkQueue.Init(VK_PHYS_DEVICE, vk::QueueFlagBits::eGraphics);

	std::vector<std::string> deviceExtensionStrings;
	for (auto& extension : VK_PHYS_DEVICE.enumerateDeviceExtensionProperties()) {
		deviceExtensionStrings.push_back(extension.extensionName);
	}
	std::vector<std::string> devicelayerStrings;
	for (auto& layer : VK_PHYS_DEVICE.enumerateDeviceLayerProperties()) {
		devicelayerStrings.push_back(layer.layerName);
	}
	std::vector<const char*> devicelayers;
	std::vector<const char*> deviceExtensions;
	//device extensions
	deviceExtensions.push_back("VK_KHR_swapchain");
	deviceExtensions.push_back("VK_KHR_dedicated_allocation");
	deviceExtensions.push_back("VK_KHR_get_memory_requirements2");
	deviceExtensions.push_back("VK_KHR_maintenance1");
	deviceExtensions.push_back("VK_KHR_sampler_mirror_clamp_to_edge");
	deviceExtensions.push_back("VK_KHR_shader_draw_parameters");
	//device layers
#ifdef _DEBUG
	devicelayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	vk::PhysicalDeviceFeatures features = VK_PHYS_DEVICE.getFeatures();
	features.samplerAnisotropy = VK_TRUE;
	features.multiDrawIndirect = VK_TRUE;
	features.multiViewport = VK_TRUE;

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &m_vkQueue.GetInfo();
	deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount = (uint32_t)devicelayers.size();
	deviceInfo.ppEnabledLayerNames = devicelayers.data();
	deviceInfo.pEnabledFeatures = &features;
	VK_DEVICE = VK_PHYS_DEVICE.createDevice(deviceInfo);
	//get queues
	*static_cast<vk::Queue*>(&m_vkQueue) = VK_DEVICE.getQueue(m_vkQueue.GetQueueIndex(), 0);

	//set up command buffers
	m_CmdBufferFactory.Init(VK_DEVICE, m_vkQueue.GetQueueIndex(), 8);
}

void GraphicsEngine::CreateSwapChain(VkSurfaceKHR surface) {
	m_VKSwapChain.Surface = surface;

	VK_PHYS_DEVICE.getSurfaceSupportKHR(m_vkQueue.GetQueueIndex(), m_VKSwapChain.Surface);

	auto formats = VK_PHYS_DEVICE.getSurfaceFormatsKHR(m_VKSwapChain.Surface);
	auto surfCapabilities = VK_PHYS_DEVICE.getSurfaceCapabilitiesKHR(m_VKSwapChain.Surface);
	auto presentModes = VK_PHYS_DEVICE.getSurfacePresentModesKHR(m_VKSwapChain.Surface);

	vk::SurfaceFormatKHR format = formats[0];
	//find out if we support srgb format
	for (auto& f : formats) {
		if (f.format == vk::Format::eB8G8R8A8Unorm) {
			format = f;
			//m_VKSwapChain.SRGB = true;
		}
	}

	vk::PresentModeKHR mode = m_VSync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
	vk::SampleCountFlagBits msaaCount = vk::SampleCountFlagBits::e1;

	m_VKSwapChain.Format = format.format;

	vk::SwapchainCreateInfoKHR swapChainInfo;
	swapChainInfo.clipped = true;
	swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainInfo.surface = m_VKSwapChain.Surface;
	swapChainInfo.imageFormat = format.format;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageColorSpace = format.colorSpace;
	swapChainInfo.imageExtent = surfCapabilities.currentExtent;
	swapChainInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapChainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage;
	swapChainInfo.minImageCount = surfCapabilities.minImageCount;
	swapChainInfo.presentMode = mode;
	swapChainInfo.preTransform = surfCapabilities.currentTransform;

	m_VKSwapChain.SwapChain = VK_DEVICE.createSwapchainKHR(swapChainInfo);

	uint32_t swapChainImageCount;
	VK_DEVICE.getSwapchainImagesKHR(m_VKSwapChain.SwapChain, &swapChainImageCount, nullptr);
	VK_DEVICE.getSwapchainImagesKHR(m_VKSwapChain.SwapChain, &swapChainImageCount, m_VKSwapChain.Images);

	for (int i = 0; i < BUFFER_COUNT; i++) {
		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.components = { vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		imageViewInfo.format = format.format;
		imageViewInfo.image = m_VKSwapChain.Images[i];
		imageViewInfo.viewType = vk::ImageViewType::e2D;
		imageViewInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
		m_VKSwapChain.ImageViews[i] = VK_DEVICE.createImageView(imageViewInfo);
	}

	//renderpass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.dependencyCount = 0;
	Vector<vk::AttachmentDescription> attachments;
	vk::AttachmentDescription attachmentDesc;
	attachmentDesc.format = format.format;
	attachmentDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;
	attachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDesc.samples = msaaCount;
	attachments.push_back(attachmentDesc);

	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();

	vk::SubpassDescription subPassDesc;
	subPassDesc.colorAttachmentCount = 1;
	subPassDesc.inputAttachmentCount = 0;
	vk::AttachmentReference colorRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };
	subPassDesc.pColorAttachments = &colorRef;
	subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subPassDesc.pDepthStencilAttachment = nullptr;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.pPreserveAttachments = nullptr;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;
	m_RenderPass = VK_DEVICE.createRenderPass(renderPassInfo);

	//framebuffers
	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.height = surfCapabilities.currentExtent.height;
	framebufferInfo.width = surfCapabilities.currentExtent.width;
	framebufferInfo.layers = 1;
	vk::ImageView fbViews1[] = { m_VKSwapChain.ImageViews[0] };
	framebufferInfo.pAttachments = fbViews1;
	framebufferInfo.renderPass = m_RenderPass;
	m_VKSwapChain.FrameBuffers[0] = VK_DEVICE.createFramebuffer(framebufferInfo);

	vk::ImageView fbViews2[] = { m_VKSwapChain.ImageViews[1] };
	framebufferInfo.pAttachments = fbViews2;
	m_VKSwapChain.FrameBuffers[1] = VK_DEVICE.createFramebuffer(framebufferInfo);

	//hdr offscreen framebuffer
	Vector<vk::Format> fboFormats;
	fboFormats.push_back(vk::Format::eR16G16B16A16Unorm);
	fboFormats.push_back(vk::Format::eD24UnormS8Uint);
	Vector<vk::ImageUsageFlags> usages;
	usages.push_back(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled);
	usages.push_back(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	m_FrameBuffer.Init(VK_DEVICE, VK_PHYS_DEVICE, m_ScreenSize, fboFormats, usages, BUFFER_COUNT);
}

void GraphicsEngine::Init(glm::vec2 windowSize, bool vsync, HWND hWnd) {
	CreateContext();

	vk::Win32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.hwnd = hWnd;
	surfaceInfo.hinstance = GetModuleHandle(nullptr);

	m_VKSwapChain.Surface = m_VKContext.Instance.createWin32SurfaceKHR(surfaceInfo);

	m_VSync = vsync;
	m_ScreenSize = windowSize;

	m_DeviceAllocator.Init(VK_DEVICE, VK_PHYS_DEVICE);

	CreateSwapChain(m_VKSwapChain.Surface);

	vk::SemaphoreCreateInfo semaphoreInfo;
	m_ImageAquiredSemaphore = VK_DEVICE.createSemaphore(semaphoreInfo);
	m_TransferComplete = VK_DEVICE.createSemaphore(semaphoreInfo);
	m_RenderCompleteSemaphore = VK_DEVICE.createSemaphore(semaphoreInfo);
	m_ImguiComplete = VK_DEVICE.createSemaphore(semaphoreInfo);

	vk::FenceCreateInfo fenceInfo;
	for (int q = 0; q < BUFFER_COUNT; ++q) {
		m_Fence[q] = VK_DEVICE.createFence(fenceInfo);
	}

	VK_FRAME_INDEX = VK_DEVICE.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;
	//viewport
	m_Viewport.width = m_FrameBuffer.GetSize().x;
	m_Viewport.height = m_FrameBuffer.GetSize().y;
	m_Viewport.minDepth = 0.0f;
	m_Viewport.maxDepth = 1.0f;
	m_Viewport.x = 0;
	m_Viewport.y = 0;
	m_Scissor.extent = { (uint32_t)windowSize.x, (uint32_t)windowSize.y };
	m_Scissor.offset = { 0,0 };
	//load pipelines
	m_Pipeline.SetDefaultVertexState(Geometry::GetVertexState());

	m_Pipeline.LoadPipelineFromFile(VK_DEVICE, "shader/filled.json", m_FrameBuffer.GetRenderPass());

	m_PerFrameBuffer = m_DeviceAllocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrameBuffer), nullptr);

	for (int q = 0; q < BUFFER_COUNT; ++q) {
		m_RenderQueues[q].Init(m_DeviceAllocator);
	}
	m_StaticRenderQueue.Init(m_DeviceAllocator);

	vk::DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.maxSets = 16 * 1024;
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eSampler, 50));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 10000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eSampledImage, 1000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageImage, 1000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageTexelBuffer, 10));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 10000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 10000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBufferDynamic, 1000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eStorageBufferDynamic, 1000));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eInputAttachment, 10));
	descPoolInfo.pPoolSizes = poolSizes.data();
	descPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	VK_DESC_POOL = VK_DEVICE.createDescriptorPool(descPoolInfo);

	MemoryBudget memBudget;
	memBudget.GeometryBudget = 0;
	memBudget.MaterialBudget = 0;
	m_Resources.Init(&VK_DEVICE, VK_PHYS_DEVICE, memBudget, m_DeviceAllocator);

	//tonemapping
	m_ToneMapping.Init(VK_DEVICE, m_ScreenSize, m_FrameBuffer, VK_DESC_POOL, m_RenderPass, m_DeviceAllocator);
	m_ShadowProgram.Init(m_VKContext, m_DeviceAllocator);

	{
		//uniform set
		vk::DescriptorSetAllocateInfo descSetAllocInfo;
		descSetAllocInfo.descriptorPool = VK_DESC_POOL;
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[0];
		VK_DEVICE.allocateDescriptorSets(&descSetAllocInfo, &m_PerFrameSet);

		//ibl set
		descSetAllocInfo.descriptorPool = VK_DESC_POOL;
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[1];
		VK_DEVICE.allocateDescriptorSets(&descSetAllocInfo, &m_IBLDescSet);

		//renderqueue sets
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[2];
		vk::DescriptorSet set;
		for (int i = 0; i < BUFFER_COUNT; ++i) {
			VK_DEVICE.allocateDescriptorSets(&descSetAllocInfo, &set);
			m_RenderQueues[i].SetDescSet(set);
		}

		VK_DEVICE.allocateDescriptorSets(&descSetAllocInfo, &set);
		m_StaticRenderQueue.SetDescSet(set);
	}

	vk::WriteDescriptorSet descWrites[6 + BUFFER_COUNT];
	uint32_t c = 0;
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = vk::DescriptorType::eUniformBuffer;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 0;
	descWrites[c].dstSet = m_PerFrameSet;
	vk::DescriptorBufferInfo descBufferInfo;
	descBufferInfo.buffer = m_PerFrameBuffer.buffer;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	descWrites[c++].pBufferInfo = &descBufferInfo;
	//ibl tex
	m_IBLTex.Init("assets/textures/ibl.dds", &m_DeviceAllocator, VK_DEVICE);
	m_SkyRad.Init("assets/textures/skybox_rad.dds", &m_DeviceAllocator, VK_DEVICE);
	m_SkyIrr.Init("assets/textures/skybox_irr.dds", &m_DeviceAllocator, VK_DEVICE);
	vk::DescriptorImageInfo imageInfo[3];
	imageInfo[0] = m_IBLTex.GetDescriptorInfo();
	imageInfo[1] = m_SkyRad.GetDescriptorInfo();
	imageInfo[2] = m_SkyIrr.GetDescriptorInfo();
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 0;
	descWrites[c].pImageInfo = &imageInfo[0];
	descWrites[c++].dstSet = m_IBLDescSet;
	descWrites[c].descriptorCount = 2;
	descWrites[c].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 1;
	descWrites[c].pImageInfo = &imageInfo[1];
	descWrites[c++].dstSet = m_IBLDescSet;
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 2;
	descWrites[c].pImageInfo = &m_ShadowProgram.GetFrameBuffer().GetDescriptors()[0];
	descWrites[c++].dstSet = m_IBLDescSet;

	vk::DescriptorBufferInfo bufferInfos[BUFFER_COUNT + 1];
	for (int q = 0; q < BUFFER_COUNT; ++q) {
		descWrites[c + q].descriptorCount = 1;
		descWrites[c + q].descriptorType = vk::DescriptorType::eStorageBuffer;
		descWrites[c + q].dstArrayElement = 0;
		descWrites[c + q].dstBinding = 0;
		descWrites[c + q].dstSet = m_RenderQueues[q].GetDescriptorSet();

		bufferInfos[q].buffer = m_RenderQueues[q].GetUniformBuffer().buffer;
		bufferInfos[q].offset = 0;
		bufferInfos[q].range = VK_WHOLE_SIZE;
		descWrites[c + q].pBufferInfo = &bufferInfos[q];
	}

	descWrites[c + 2].descriptorCount = 1;
	descWrites[c + 2].descriptorType = vk::DescriptorType::eStorageBuffer;
	descWrites[c + 2].dstArrayElement = 0;
	descWrites[c + 2].dstBinding = 0;
	descWrites[c + 2].dstSet = m_StaticRenderQueue.GetDescriptorSet();

	bufferInfos[2].buffer = m_StaticRenderQueue.GetUniformBuffer().buffer;
	bufferInfos[2].offset = 0;
	bufferInfos[2].range = VK_WHOLE_SIZE;
	descWrites[c + 2].pBufferInfo = &bufferInfos[2];

	VK_DEVICE.updateDescriptorSets(c + BUFFER_COUNT + 1, descWrites, 0, nullptr);

	m_SkyBox.Init(VK_DEVICE, VK_PHYS_DEVICE, "assets/textures/skybox.dds", m_Viewport, m_FrameBuffer.GetRenderPass(), m_DeviceAllocator);
	//prepare initial transfer
	auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
	cmdBuffer.Begin(nullptr, nullptr);

	for (int i = 0; i < BUFFER_COUNT; i++) {
		cmdBuffer.ImageBarrier(m_VKSwapChain.Images[i], vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

		Vector<vk::ImageLayout> layouts;
		layouts.push_back(vk::ImageLayout::eColorAttachmentOptimal);
		layouts.push_back(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		m_FrameBuffer.ChangeLayout(cmdBuffer, layouts, i);
	}

	cmdBuffer.PushPipelineBarrier();
	m_DeviceAllocator.ScheduleTransfers(&cmdBuffer);
	m_CmdBufferFactory.EndBuffer(cmdBuffer);

	m_vkQueue.Submit(cmdBuffer);
	VK_DEVICE.waitIdle();

	pipelineEditor.Init(VK_DEVICE);
	pipelineEditor.Open("shader/filled.json");
}

void GraphicsEngine::RenderModels(RenderQueue& rq, CommandBuffer& cmdBuffer) {
	if (rq.GetModels().size() == 0) {
		return;
	}
	vk::DescriptorSet sets[] = { m_PerFrameSet, m_IBLDescSet, rq.GetDescriptorSet() };
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
	auto& models = rq.GetModels();
	for (auto& m : models) {
		uint32_t instanceCount = m.second.Count;
		m_Stats.ModelCount += instanceCount;

		const Model& model = m_Resources.GetModel(m.first);
		const vk::Buffer vertexBuffers[4] = { model.VertexBuffers[0].buffer, model.VertexBuffers[1].buffer,
		                                      model.VertexBuffers[2].buffer, model.VertexBuffers[3].buffer
		                                    };

		const vk::DeviceSize offsets[4] = { 0,0,0,0 };
		cmdBuffer.bindVertexBuffers(0, 4, vertexBuffers, offsets);
		cmdBuffer.bindIndexBuffer(model.IndexBuffer.buffer, 0, vk::IndexType::eUint16);
		cmdBuffer.pushConstants(m_Pipeline.GetPipelineLayout(), vk::ShaderStageFlagBits::eAll, 0, sizeof(unsigned), &m.second.Offset);
		vk::DescriptorSet mat;
		for (uint32_t meshIt = 0; meshIt < model.MeshCount; ++meshIt) {
			m_Stats.MeshCount += instanceCount;
			Mesh& mesh = model.Meshes[meshIt];
			if (mat != mesh.Material) {
				mat = mesh.Material;
				cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipelineLayout(), 3, 1, &mesh.Material, 0, nullptr);
			}
			m_Stats.TriangleCount += (mesh.IndexCount / 3) * instanceCount;
			cmdBuffer.drawIndexed(mesh.IndexCount, instanceCount, mesh.IndexOffset, 0, 0);
		}
	}
}

void GraphicsEngine::Render() {

	pipelineEditor.Update();
	RenderQueue& rq = m_RenderQueues[VK_FRAME_INDEX];
	//reset stats
	{
		m_Stats.ModelCount = 0;
		m_Stats.MeshCount = 0;
		m_Stats.TriangleCount = 0;
	}

	if (ImGui::Button("Recompile Shader")) {
		m_ShadowProgram.Recompile();
		m_Pipeline.ReloadPipelineFromFile(VK_DEVICE, "shader/filled.json", m_RenderPass);
	}
	//Transfer new frame data to GPU
	{
		const CameraData& cd = rq.GetCameras()[0];

		m_ShadowProgram.Update(m_DeviceAllocator, rq);

		PerFrameBuffer uniformBuffer;
		static int shadowIndex = 0;
		uniformBuffer.ViewProj = cd.ProjView;
		uniformBuffer.View = cd.View;
		uniformBuffer.CameraPos = glm::vec4(cd.Position, 1);
		static glm::vec3 lightDir = glm::vec3(0.1f, -1.0f, -0.5f);
		static float globalRoughness = 1.0f;
		static bool metallic = false;
		//ImGui::Begin("Scene");
		//ImGui::DragFloat3("Lighting direction", &lightDir[0], 0.001f, -1.0f, 1.0f);
		//ImGui::SliderFloat("Global Roughness", &globalRoughness,0, 1.0f);
		//ImGui::Checkbox("Metal", &metallic);
		//ImGui::End();
		uniformBuffer.LightDir = glm::normalize(glm::vec4(lightDir, 1.0f));
		uniformBuffer.Material.x = globalRoughness;
		uniformBuffer.Material.y = metallic ? 1.0f : 0.0f;
		uniformBuffer.LightViewProj[0] = m_ShadowProgram.GetShadowMatrix(0);
		uniformBuffer.LightViewProj[1] = m_ShadowProgram.GetShadowMatrix(1);
		uniformBuffer.LightViewProj[2] = m_ShadowProgram.GetShadowMatrix(2);
		uniformBuffer.LightViewProj[3] = m_ShadowProgram.GetShadowMatrix(3);
		uniformBuffer.NearFarPadding = glm::vec4(cd.Near, cd.Far, 0, 0);
		uniformBuffer.ShadowSplits = m_ShadowProgram.GetShadowSplits();

		m_DeviceAllocator.UpdateBuffer(m_PerFrameBuffer, sizeof(PerFrameBuffer), &uniformBuffer);
		m_ToneMapping.Update(m_DeviceAllocator);
		rq.ScheduleTransfer(m_DeviceAllocator);

		m_CmdBufferFactory.Reset(VK_DEVICE, VK_FRAME_INDEX);

		auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer.Begin(nullptr, nullptr);
		m_DeviceAllocator.ScheduleTransfers(&cmdBuffer);
		m_SkyBox.PrepareUniformBuffer(cmdBuffer, m_DeviceAllocator, cd.ProjView, glm::translate(cd.Position));
		m_CmdBufferFactory.EndBuffer(cmdBuffer);

		m_vkQueue.Submit(cmdBuffer, nullptr, m_TransferComplete, nullptr);

	}

	std::vector<vk::CommandBuffer> cmdBuffers;
	//render pass
	{
		auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();

		m_ShadowProgram.Render(VK_FRAME_INDEX, cmdBuffer, rq, m_Resources);
		m_CmdBufferFactory.EndBuffer(cmdBuffer);
		cmdBuffers.push_back(cmdBuffer);

		cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer.Begin(m_FrameBuffer.GetFrameBuffer(VK_FRAME_INDEX), m_FrameBuffer.GetRenderPass());

		Vector<vk::ImageLayout> newLayouts;
		newLayouts.push_back(vk::ImageLayout::eColorAttachmentOptimal);
		newLayouts.push_back(vk::ImageLayout::eDepthStencilAttachmentOptimal);

		m_FrameBuffer.ChangeLayout(cmdBuffer, newLayouts, VK_FRAME_INDEX);
		cmdBuffer.PushPipelineBarrier();

		vk::RenderPassBeginInfo renderPassInfo;
		renderPassInfo.framebuffer = m_FrameBuffer.GetFrameBuffer(VK_FRAME_INDEX);
		renderPassInfo.renderPass = m_FrameBuffer.GetRenderPass();
		glm::vec4 color = glm::vec4(0);
		vk::ClearColorValue clearColor;
		clearColor.float32[0] = color.r;
		clearColor.float32[1] = color.g;
		clearColor.float32[2] = color.b;
		clearColor.float32[3] = 1;
		vk::ClearDepthStencilValue clearDepth;
		clearDepth.depth = 1.0f;
		clearDepth.stencil = 0x0;
		vk::ClearValue clearValues[] = { clearColor, clearDepth };
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;
		renderPassInfo.renderArea = { 0, 0, (uint32_t)m_FrameBuffer.GetSize().x, (uint32_t)m_FrameBuffer.GetSize().y };
		cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		cmdBuffer.setViewport(0, 1, &m_Viewport);
		cmdBuffer.setScissor(0, 1, &m_Scissor);
		//skybox
		m_SkyBox.Render(cmdBuffer);
		//render here
		cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipeline());

		RenderModels(m_StaticRenderQueue, cmdBuffer);
		RenderModels(rq, cmdBuffer);

		cmdBuffer.endRenderPass();

		m_CmdBufferFactory.EndBuffer(cmdBuffer);
		cmdBuffers.push_back(cmdBuffer);

		m_vkQueue.Submit(cmdBuffers, { m_TransferComplete, m_ImageAquiredSemaphore }, { m_RenderCompleteSemaphore }, nullptr);
	}
	//Perform tonemapping and render Imgui
	{
		auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer.Begin(m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX], m_RenderPass);
		// dont trust the renderpass inital layout
		cmdBuffer.ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal);
		cmdBuffer.PushPipelineBarrier();

		vk::RenderPassBeginInfo rpBeginInfo;
		rpBeginInfo.clearValueCount = 1;
		rpBeginInfo.renderPass = m_RenderPass;
		rpBeginInfo.framebuffer = m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX];
		rpBeginInfo.renderArea.extent.width = (uint32_t)m_ScreenSize.x;
		rpBeginInfo.renderArea.extent.height = (uint32_t)m_ScreenSize.y;
		vk::ClearColorValue cc;
		cc.float32[0] = 0.5f;
		cc.float32[1] = 0.5f;
		cc.float32[2] = 1.0f;
		cc.float32[3] = 1.0f;
		vk::ClearValue cv = { cc };
		rpBeginInfo.pClearValues = &cv;

		Vector<vk::ImageLayout> newLayouts;
		newLayouts.push_back(vk::ImageLayout::eShaderReadOnlyOptimal);
		newLayouts.push_back(vk::ImageLayout::eDepthStencilReadOnlyOptimal);

		m_FrameBuffer.ChangeLayout(cmdBuffer, newLayouts, VK_FRAME_INDEX);
		cmdBuffer.PushPipelineBarrier();
		cmdBuffer.beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);
		cmdBuffer.setViewport(0, 1, &m_Viewport);
		cmdBuffer.setScissor(0, 1, &m_Scissor);
		//transfer from fbo to back buffer
		m_ToneMapping.Render(cmdBuffer, m_Viewport, VK_FRAME_INDEX);
		//render imgui on top
		ImGui_ImplGlfwVulkan_Render(cmdBuffer);
		cmdBuffer.endRenderPass();

		m_CmdBufferFactory.EndBuffer(cmdBuffer);

		m_vkQueue.Submit(cmdBuffer, m_RenderCompleteSemaphore, m_ImguiComplete, m_Fence[VK_FRAME_INDEX]);
	}
}

void GraphicsEngine::Swap() {

	VK_DEVICE.waitForFences(1, &m_Fence[VK_FRAME_INDEX], true, UINT64_MAX);
	VK_DEVICE.resetFences(1, &m_Fence[VK_FRAME_INDEX]);

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.pImageIndices = &VK_FRAME_INDEX;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_VKSwapChain.SwapChain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_ImguiComplete;
	m_vkQueue.presentKHR(presentInfo);

	m_RenderQueues[VK_FRAME_INDEX].Clear();

	VK_FRAME_INDEX = VK_DEVICE.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;

	m_DeviceAllocator.Clear();
}

RenderQueue* GraphicsEngine::GetRenderQueue() {
	return &m_RenderQueues[VK_FRAME_INDEX];
}

RenderQueue* GraphicsEngine::GetStaticQueue() {
	return &m_StaticRenderQueue;
}

void GraphicsEngine::TransferToGPU() {
	auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
	cmdBuffer.Begin(nullptr, nullptr);
	m_Resources.ScheduleTransfer(cmdBuffer);
	m_CmdBufferFactory.EndBuffer(cmdBuffer);
	m_StaticRenderQueue.ScheduleTransfer(m_DeviceAllocator);
	m_vkQueue.Submit(cmdBuffer);
	VK_DEVICE.waitIdle();
}

#ifdef USE_IMGUI
void check_vk_result(VkResult err) {
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

ImGui_ImplGlfwVulkan_Init_Data* GraphicsEngine::GetImguiInit() {
	ImGui_ImplGlfwVulkan_Init_Data* id = new ImGui_ImplGlfwVulkan_Init_Data();
	id->check_vk_result = check_vk_result;
	id->descriptor_pool = VK_DESC_POOL;
	id->device = VK_DEVICE;
	id->gpu = VK_PHYS_DEVICE;
	id->render_pass = m_RenderPass;
	id->allocator = nullptr;
	id->pipeline_cache = nullptr;
	return id;
}

void GraphicsEngine::CreateImguiFont(ImGuiContext* imguiCtx) {
	m_ImguiCtx = imguiCtx;
	ImGui::SetCurrentContext(imguiCtx);
	//upload font texture
	{
		auto& cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		vk::CommandBufferBeginInfo begin_info = {};
		begin_info.flags |= vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
		cmdBuffer.begin(&begin_info);

		ImGui_ImplGlfwVulkan_CreateFontsTexture(cmdBuffer);

		m_CmdBufferFactory.EndBuffer(cmdBuffer);
		m_vkQueue.Submit(cmdBuffer);

		vkDeviceWaitIdle(VK_DEVICE);
		ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
	}
}
#endif


void GraphicsEngine::PrintStats() {
	static bool frameStats = false;
	if (g_Input.IsKeyPushed(GLFW_KEY_F10)) {
		frameStats = !frameStats;
	}
	if (frameStats) {
		ImGui::Begin("FrameStats");
		ImGui::Text("ModelCount: %u", m_Stats.ModelCount);
		ImGui::Text("MeshCount: %u", m_Stats.MeshCount);
		ImGui::Text("TriangleCount: %u", m_Stats.TriangleCount);
		ImGui::End();
	}
}