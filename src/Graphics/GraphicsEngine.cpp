#include "GraphicsEngine.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "Vertex.h"
#include "Core/Timer.h"
#include "Core/Input.h"
#define USE_DEBUG_LAYER

using namespace smug;

GraphicsEngine::GraphicsEngine() {

}

GraphicsEngine::~GraphicsEngine() {
	m_DeviceAllocator.Clear();
	for (int i = 0; i < BUFFER_COUNT; ++i) {
		m_RenderQueues[i].Destroy(m_Resources);
	}
	m_StaticRenderQueue.Destroy(m_Resources);
	
	m_Resources.Clear();
	m_DeviceAllocator.DeAllocateBuffer(m_PerFrameBuffer);
	m_DeviceAllocator.DeAllocateImage(m_IBLTex.GetImageHandle());
	m_DeviceAllocator.DeAllocateImage(m_SkyIrr.GetImageHandle());
	m_DeviceAllocator.DeAllocateImage(m_SkyRad.GetImageHandle());
	m_SkyBox.DeInit(m_DeviceAllocator);
	m_ShadowProgram.DeInit(m_DeviceAllocator);
	m_ToneMapping.DeInit(m_DeviceAllocator);
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
#ifdef _DEBUG
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT || messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		DebugBreak();
	}
#endif
	return VK_FALSE;
}

void GraphicsEngine::CreateContext() {

	volkInitialize();

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "SmugEngine";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "SmugEngine";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_1;

	std::vector<const char*> layers;
	std::vector<const char*> extensions;

#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	extensions.push_back("VK_EXT_debug_utils");
	//layers.push_back("VK_LAYER_RENDERDOC_Capture");
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
	VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo = {};
	messengerCreateInfo.flags = 0;
	messengerCreateInfo.messageSeverity =	VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
	                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
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

	VkInstanceCreateInfo instInfo = {};
	instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instInfo.pNext = nullptr;
	instInfo.enabledExtensionCount = (uint32_t)extensions.size();
	instInfo.pApplicationInfo = &appInfo;
	instInfo.ppEnabledExtensionNames = extensions.data();
	instInfo.enabledLayerCount = (uint32_t)layers.size();
	instInfo.ppEnabledLayerNames = layers.data();
#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	//push into instance create info to detect problem during initialization
	instInfo.pNext = &messengerCreateInfo;
#endif
	vkCreateInstance(&instInfo, nullptr, &m_VKContext.Instance);
	volkLoadInstance(m_VKContext.Instance);

#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	VkDebugUtilsMessengerEXT messenger;
	//PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)m_VKContext.Instance.getProcAddr("vkCreateDebugUtilsMessengerEXT");
	vkCreateDebugUtilsMessengerEXT(m_VKContext.Instance, &messengerCreateInfo, nullptr, &messenger);
#endif
	uint32_t gpuCount = 0;
	vkEnumeratePhysicalDevices(m_VKContext.Instance, &gpuCount, nullptr);
	std::vector<VkPhysicalDevice> gpus(gpuCount);
	vkEnumeratePhysicalDevices(m_VKContext.Instance, &gpuCount, &gpus[0]);
	if (gpus.size() == 0) {
		return;
	}
	//grab first available GPU
	for (auto& gpu : gpus) {
		VkPhysicalDeviceProperties gpuProps;
		vkGetPhysicalDeviceProperties(gpu, &gpuProps);
		if (gpuProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ||
			gpuProps.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
			VK_PHYS_DEVICE = gpu;
			break;
		}
	}

	m_vkQueue.Init(VK_PHYS_DEVICE, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);

	std::vector<const char*> devicelayers;
	std::vector<const char*> deviceExtensions;
	//device extensions
	deviceExtensions.push_back("VK_KHR_swapchain");
	deviceExtensions.push_back("VK_KHR_dedicated_allocation");
	deviceExtensions.push_back("VK_KHR_get_memory_requirements2");
	deviceExtensions.push_back("VK_KHR_sampler_mirror_clamp_to_edge");
#if defined(RTX_ON)
	deviceExtensions.push_back("VK_EXT_descriptor_indexing");
	deviceExtensions.push_back("VK_NV_ray_tracing");
#endif
	//device layers
#if defined(_DEBUG) && defined(USE_DEBUG_LAYER)
	devicelayers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(VK_PHYS_DEVICE, &features);
	features.samplerAnisotropy = VK_TRUE;
	features.multiDrawIndirect = VK_TRUE;
	features.multiViewport = VK_TRUE;
	features.wideLines = VK_TRUE;

	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &m_vkQueue.GetInfo();
	deviceInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();
	deviceInfo.enabledLayerCount = (uint32_t)devicelayers.size();
	deviceInfo.ppEnabledLayerNames = devicelayers.data();
	deviceInfo.pEnabledFeatures = &features;
	vkCreateDevice(VK_PHYS_DEVICE, &deviceInfo, nullptr, &VK_DEVICE);
	//get queues
	m_vkQueue.SetQueue(VK_DEVICE);

	//set up command buffers
	m_CmdBufferFactory.Init(VK_DEVICE, m_vkQueue.GetQueueIndex(), 8);
}

void GraphicsEngine::CreateSwapChain(VkSurfaceKHR surface) {
	m_VKSwapChain.Surface = surface;

	VkBool32 supported = false;
	vkGetPhysicalDeviceSurfaceSupportKHR(VK_PHYS_DEVICE, m_vkQueue.GetQueueIndex(), surface, &supported);
	if (!supported) {
		printf("Unsupported surface\n");
		return;
	}
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(VK_PHYS_DEVICE, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(VK_PHYS_DEVICE, surface, &formatCount, &formats[0]);
	VkSurfaceFormatKHR format = formats[0];
	for (auto& f : formats) {
		if (f.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM) {
			format = f;
		}
	}

	VkPresentModeKHR mode = m_VSync ? VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR : VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
	VkSampleCountFlagBits msaaCount = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

	m_VKSwapChain.Format = format.format;
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VK_PHYS_DEVICE, surface, &surfaceCaps);

	VkSwapchainCreateInfoKHR swapChainInfo = {};
	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.pNext = nullptr;
	swapChainInfo.clipped = true;
	swapChainInfo.compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.surface = m_VKSwapChain.Surface;
	swapChainInfo.imageFormat = format.format;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageColorSpace = format.colorSpace;
	swapChainInfo.imageExtent = surfaceCaps.currentExtent;
	swapChainInfo.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
	swapChainInfo.minImageCount = surfaceCaps.minImageCount;
	swapChainInfo.presentMode = mode;
	swapChainInfo.preTransform = surfaceCaps.currentTransform;
	vkCreateSwapchainKHR(VK_DEVICE, &swapChainInfo, nullptr, &m_VKSwapChain.SwapChain);

	uint32_t swapChainImageCount;
	vkGetSwapchainImagesKHR(VK_DEVICE, m_VKSwapChain.SwapChain, &swapChainImageCount, nullptr);
	VkImage swapChainImages[2];
	vkGetSwapchainImagesKHR(VK_DEVICE, m_VKSwapChain.SwapChain, &swapChainImageCount, swapChainImages);
	m_VKSwapChain.Images[0] = swapChainImages[0];
	m_VKSwapChain.Images[1] = swapChainImages[1];

	for (int i = 0; i < BUFFER_COUNT; i++) {
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.components = { VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B, VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A };
		imageViewInfo.format = format.format;
		imageViewInfo.image = m_VKSwapChain.Images[i];
		imageViewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.subresourceRange = { VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		vkCreateImageView(VK_DEVICE, &imageViewInfo, nullptr, &m_VKSwapChain.ImageViews[i]);
	}

	//renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.dependencyCount = 0;
	std::vector<VkAttachmentDescription> attachments;
	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.format = format.format;
	attachmentDesc.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDesc.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDesc.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.samples = msaaCount;
	attachments.push_back(attachmentDesc);

	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();

	VkSubpassDescription subPassDesc = {};
	subPassDesc.colorAttachmentCount = 1;
	subPassDesc.inputAttachmentCount = 0;
	VkAttachmentReference colorRef = { 0, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	subPassDesc.pColorAttachments = &colorRef;
	subPassDesc.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPassDesc.pDepthStencilAttachment = nullptr;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.pPreserveAttachments = nullptr;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;
	vkCreateRenderPass(VK_DEVICE, &renderPassInfo, nullptr, &m_RenderPass);

	//framebuffers
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = nullptr;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.height = surfaceCaps.currentExtent.height;
	framebufferInfo.width = surfaceCaps.currentExtent.width;
	framebufferInfo.layers = 1;
	VkImageView fbViews1[] = { m_VKSwapChain.ImageViews[0] };
	framebufferInfo.pAttachments = fbViews1;
	framebufferInfo.renderPass = m_RenderPass;
	vkCreateFramebuffer(VK_DEVICE, &framebufferInfo, nullptr, &m_VKSwapChain.FrameBuffers[0]);

	VkImageView fbViews2[] = { m_VKSwapChain.ImageViews[1] };
	framebufferInfo.pAttachments = fbViews2;
	vkCreateFramebuffer(VK_DEVICE, &framebufferInfo, nullptr, &m_VKSwapChain.FrameBuffers[1]);


	//hdr offscreen framebuffer
	std::vector<VkFormat> fboFormats;
	fboFormats.push_back(VkFormat::VK_FORMAT_R16G16B16A16_UNORM);
	fboFormats.push_back(VkFormat::VK_FORMAT_D24_UNORM_S8_UINT);
	std::vector<VkImageUsageFlags> usages;
	usages.push_back(VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);
	usages.push_back(VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);
	m_FrameBuffer.Init(VK_DEVICE, VK_PHYS_DEVICE, m_ScreenSize, fboFormats, usages);
}

void GraphicsEngine::Init(glm::vec2 windowSize, bool vsync, HWND hWnd) {
	CreateContext();

	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.hwnd = hWnd;
	surfaceInfo.hinstance = GetModuleHandle(nullptr);
	vkCreateWin32SurfaceKHR(m_VKContext.Instance, &surfaceInfo, nullptr, &m_VKSwapChain.Surface);

	m_VSync = vsync;
	m_ScreenSize = windowSize;

	m_DeviceAllocator.Init(VK_DEVICE, VK_PHYS_DEVICE);

	CreateSwapChain(m_VKSwapChain.Surface);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;
	vkCreateSemaphore(VK_DEVICE, &semaphoreInfo, nullptr, &m_ImageAquiredSemaphore);
	vkCreateSemaphore(VK_DEVICE, &semaphoreInfo, nullptr, &m_TransferComplete);
	vkCreateSemaphore(VK_DEVICE, &semaphoreInfo, nullptr, &m_RenderCompleteSemaphore);
	vkCreateSemaphore(VK_DEVICE, &semaphoreInfo, nullptr, &m_ImguiComplete);

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	for (int q = 0; q < BUFFER_COUNT; ++q) {
		vkCreateFence(VK_DEVICE, &fenceInfo, nullptr, &m_Fence[q]);
	}

	vkAcquireNextImageKHR(VK_DEVICE, m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr, &VK_FRAME_INDEX);
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

	MemoryBudget memBudget;
	memBudget.GeometryBudget = 0;
	memBudget.MaterialBudget = 0;
	m_Resources.Init(&VK_DEVICE, VK_PHYS_DEVICE, memBudget, m_DeviceAllocator, m_FrameBuffer);

	for (int q = 0; q < BUFFER_COUNT; ++q) {
		m_RenderQueues[q].Init(m_Resources, q);
	}
	m_StaticRenderQueue.Init(m_Resources, BUFFER_COUNT);

	VkDescriptorPoolCreateInfo descPoolInfo = {};
	descPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descPoolInfo.pNext = nullptr;
	descPoolInfo.maxSets = 16 * 1024;
	std::vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER, 50 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 });
	poolSizes.push_back({ VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10 });
	descPoolInfo.pPoolSizes = poolSizes.data();
	descPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	vkCreateDescriptorPool(VK_DEVICE, &descPoolInfo, nullptr, &VK_DESC_POOL);

	//tonemapping
	m_ToneMapping.Init(VK_DEVICE, m_ScreenSize, m_FrameBuffer, VK_DESC_POOL, m_RenderPass, m_DeviceAllocator);
	m_ShadowProgram.Init(m_VKContext, m_DeviceAllocator);
#if defined(RTX_ON)
	m_RaytracingProgram.Init(VK_DEVICE, VK_PHYS_DEVICE, m_VKContext.Instance, m_VKContext.DescriptorPool, m_FrameBuffer.GetView(0, 0), &m_DeviceAllocator);
#endif
	{
		//uniform set
		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.pNext = nullptr;
		descSetAllocInfo.descriptorPool = VK_DESC_POOL;
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[0];
		vkAllocateDescriptorSets(VK_DEVICE, &descSetAllocInfo, &m_PerFrameSet);
		//ibl set
		descSetAllocInfo.descriptorPool = VK_DESC_POOL;
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[1];
		vkAllocateDescriptorSets(VK_DEVICE, &descSetAllocInfo, &m_IBLDescSet);
		//renderqueue sets
		descSetAllocInfo.descriptorSetCount = 1;
		descSetAllocInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayouts()[2];
		VkDescriptorSet set;
		for (int i = 0; i < BUFFER_COUNT; ++i) {
			vkAllocateDescriptorSets(VK_DEVICE, &descSetAllocInfo, &set);
			m_RenderQueues[i].SetDescSet(set);
		}
		vkAllocateDescriptorSets(VK_DEVICE, &descSetAllocInfo, &set);
		m_StaticRenderQueue.SetDescSet(set);
	}

	VkWriteDescriptorSet descWrites[6 + BUFFER_COUNT];
	uint32_t c = 0;
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 0;
	descWrites[c].dstSet = m_PerFrameSet;
	VkDescriptorBufferInfo descBufferInfo;
	descBufferInfo.buffer = m_PerFrameBuffer.buffer;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	descWrites[c++].pBufferInfo = &descBufferInfo;
	//ibl tex
	m_IBLTex.Init("assets/textures/ibl.dds", &m_DeviceAllocator, VK_DEVICE);
	m_SkyRad.Init("assets/textures/skybox_rad.dds", &m_DeviceAllocator, VK_DEVICE);
	m_SkyIrr.Init("assets/textures/skybox_irr.dds", &m_DeviceAllocator, VK_DEVICE);
	VkDescriptorImageInfo imageInfo[3];
	imageInfo[0] = m_IBLTex.GetDescriptorInfo();
	imageInfo[1] = m_SkyRad.GetDescriptorInfo();
	imageInfo[2] = m_SkyIrr.GetDescriptorInfo();
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 0;
	descWrites[c].pImageInfo = &imageInfo[0];
	descWrites[c++].dstSet = m_IBLDescSet;
	descWrites[c].descriptorCount = 2;
	descWrites[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 1;
	descWrites[c].pImageInfo = &imageInfo[1];
	descWrites[c++].dstSet = m_IBLDescSet;
	descWrites[c].descriptorCount = 1;
	descWrites[c].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descWrites[c].dstArrayElement = 0;
	descWrites[c].dstBinding = 2;
	descWrites[c].pImageInfo = &m_ShadowProgram.GetFrameBuffer().GetDescriptors()[0];
	descWrites[c++].dstSet = m_IBLDescSet;

	VkDescriptorBufferInfo bufferInfos[BUFFER_COUNT + 1];
	for (int q = 0; q < BUFFER_COUNT; ++q) {
		descWrites[c + q].descriptorCount = 1;
		descWrites[c + q].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descWrites[c + q].dstArrayElement = 0;
		descWrites[c + q].dstBinding = 0;
		descWrites[c + q].dstSet = m_RenderQueues[q].GetDescriptorSet();

		bufferInfos[q].buffer = m_RenderQueues[q].GetUniformBuffer().buffer;
		bufferInfos[q].offset = 0;
		bufferInfos[q].range = VK_WHOLE_SIZE;
		descWrites[c + q].pBufferInfo = &bufferInfos[q];
	}

	descWrites[c + 2].descriptorCount = 1;
	descWrites[c + 2].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	descWrites[c + 2].dstArrayElement = 0;
	descWrites[c + 2].dstBinding = 0;
	descWrites[c + 2].dstSet = m_StaticRenderQueue.GetDescriptorSet();

	bufferInfos[2].buffer = m_StaticRenderQueue.GetUniformBuffer().buffer;
	bufferInfos[2].offset = 0;
	bufferInfos[2].range = VK_WHOLE_SIZE;
	descWrites[c + 2].pBufferInfo = &bufferInfos[2];

	for (uint32_t i = 0; i < 6 + BUFFER_COUNT; ++i) {
		descWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descWrites[i].pNext = nullptr;
	}

	vkUpdateDescriptorSets(VK_DEVICE, c + BUFFER_COUNT + 1, descWrites, 0, nullptr);
	m_SkyBox.Init(VK_DEVICE, VK_PHYS_DEVICE, "assets/textures/skybox.dds", m_Viewport, m_FrameBuffer.GetRenderPass(), m_DeviceAllocator);
	//prepare initial transfer
	CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
	cmdBuffer->Begin(nullptr, nullptr);

	for (int i = 0; i < BUFFER_COUNT; i++) {
		cmdBuffer->ImageBarrier(m_VKSwapChain.Images[i], VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}

	std::vector<VkImageLayout> layouts;
	layouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	layouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
	m_FrameBuffer.ChangeLayout(*cmdBuffer, layouts);
	m_DeviceAllocator.ScheduleTransfers(cmdBuffer);
	
#if defined(RTX_ON)
	m_RaytracingProgram.Update(cmdBuffer, &m_DeviceAllocator);
#endif

	m_CmdBufferFactory.EndBuffer(cmdBuffer);
	m_vkQueue.Submit(cmdBuffer->CmdBuffer());
	//m_vkQueue.waitIdle();
	vkQueueWaitIdle(m_vkQueue.GetQueue());

	//pipelineEditor.Init(VK_DEVICE);
	//pipelineEditor.Open("shader/filled.json");

	m_Profiler.Init(VK_DEVICE, VK_PHYS_DEVICE);
}

void GraphicsEngine::RenderModels(RenderQueue& rq, CommandBuffer& cmdBuffer) {
	if (rq.GetModels().size() == 0) {
		return;
	}
	VkDescriptorSet sets[] = { m_PerFrameSet, m_IBLDescSet, rq.GetDescriptorSet() };
	vkCmdBindDescriptorSets(cmdBuffer.CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
	auto& models = rq.GetModels();
	for (auto& m : models) {
		uint32_t instanceCount = m.second.Count;
		m_Stats.ModelCount += instanceCount;

		const Model& model = m_Resources.GetModel(m.first);
		const VkBuffer vertexBuffers[4] = { model.VertexBuffers[0].buffer, model.VertexBuffers[1].buffer,
		                                      model.VertexBuffers[2].buffer, model.VertexBuffers[3].buffer
		                                    };
		const VkDeviceSize offsets[4] = { 0,0,0,0 };
		vkCmdBindVertexBuffers(cmdBuffer.CmdBuffer(), 0, 4, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(cmdBuffer.CmdBuffer(), model.IndexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(cmdBuffer.CmdBuffer(), m_Pipeline.GetPipelineLayout(), VkShaderStageFlagBits::VK_SHADER_STAGE_ALL, 0, sizeof(unsigned), &m.second.Offset);
		VkDescriptorSet mat = nullptr;
		for (uint32_t meshIt = 0; meshIt < model.MeshCount; ++meshIt) {
			m_Stats.MeshCount += instanceCount;
			Mesh& mesh = model.Meshes[meshIt];
			if (mat != mesh.Material.DescSet) {
				mat = mesh.Material.DescSet;
				vkCmdBindDescriptorSets(cmdBuffer.CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 3, 1, &mesh.Material.DescSet, 0, nullptr);
			}
			m_Stats.TriangleCount += (mesh.IndexCount / 3) * instanceCount;
			vkCmdDrawIndexed(cmdBuffer.CmdBuffer(), mesh.IndexCount, instanceCount, mesh.IndexOffset, 0, 0);
		}
	}
}

void GraphicsEngine::Render() {
	m_Profiler.Print();
	//pipelineEditor.Update();
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

		CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer->Begin(nullptr, nullptr);
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "Start");
		m_SkyBox.PrepareUniformBuffer(m_DeviceAllocator, cd.ProjView, glm::translate(cd.Position));
		m_DeviceAllocator.ScheduleTransfers(cmdBuffer);
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "Transfer frame data");
		m_CmdBufferFactory.EndBuffer(cmdBuffer);

		m_vkQueue.Submit(cmdBuffer->CmdBuffer(), nullptr, m_TransferComplete, nullptr);

	}

	std::vector<VkCommandBuffer> cmdBuffers;
	//render pass
	{
		//CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		//m_ShadowProgram.Render(VK_FRAME_INDEX, *cmdBuffer, rq, m_Resources, m_Profiler);
		//m_CmdBufferFactory.EndBuffer(cmdBuffer);
		//cmdBuffers.push_back(*cmdBuffer);

		CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer->Begin(m_FrameBuffer.GetFrameBuffer(), m_FrameBuffer.GetRenderPass());

		std::vector<VkImageLayout> newLayouts;
		newLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		newLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

		m_FrameBuffer.ChangeLayout(*cmdBuffer, newLayouts);
		cmdBuffer->PushPipelineBarrier();

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.framebuffer = m_FrameBuffer.GetFrameBuffer();
		renderPassInfo.renderPass = m_FrameBuffer.GetRenderPass();
		glm::vec4 color = glm::vec4(0);
		VkClearValue clearValues[2];
		clearValues[0].color.float32[0] = color.r;
		clearValues[0].color.float32[1] = color.g;
		clearValues[0].color.float32[2] = color.b;
		clearValues[0].color.float32[3] = 1;
		clearValues[1].depthStencil.depth = 1.0f;
		clearValues[1].depthStencil.stencil = 0x0;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearValues;
		renderPassInfo.renderArea = { 0, 0, (uint32_t)m_FrameBuffer.GetSize().x, (uint32_t)m_FrameBuffer.GetSize().y };
		vkCmdBeginRenderPass(cmdBuffer->CmdBuffer(), &renderPassInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(cmdBuffer->CmdBuffer(), 0, 1, &m_Viewport);
		vkCmdSetScissor(cmdBuffer->CmdBuffer(), 0, 1, &m_Scissor);
		//skybox
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "SkyBox");
		m_SkyBox.Render(cmdBuffer);
		//render here
		vkCmdBindPipeline(cmdBuffer->CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipeline());
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "Render Statics");
		RenderModels(m_StaticRenderQueue, *cmdBuffer);
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "Render Dynamics");
		RenderModels(rq, *cmdBuffer);
		vkCmdEndRenderPass(cmdBuffer->CmdBuffer());
#if defined(RTX_ON)
		//render raytraced geometry
		m_RaytracingProgram.Render(cmdBuffer);
#endif
		m_CmdBufferFactory.EndBuffer(cmdBuffer);
		cmdBuffers.push_back(cmdBuffer->CmdBuffer());

		m_vkQueue.Submit(cmdBuffers, { m_TransferComplete, m_ImageAquiredSemaphore }, { m_RenderCompleteSemaphore }, nullptr);
	}
	//Perform tonemapping and render Imgui
	{
		CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer->Begin(m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX], m_RenderPass);
		// dont trust the renderpass inital layout
		cmdBuffer->ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
		cmdBuffer->PushPipelineBarrier();

		VkRenderPassBeginInfo rpBeginInfo = {};
		rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpBeginInfo.pNext = nullptr;
		rpBeginInfo.clearValueCount = 1;
		rpBeginInfo.renderPass = m_RenderPass;
		rpBeginInfo.framebuffer = m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX];
		rpBeginInfo.renderArea.extent.width = (uint32_t)m_ScreenSize.x;
		rpBeginInfo.renderArea.extent.height = (uint32_t)m_ScreenSize.y;
		VkClearColorValue cc;
		cc.float32[0] = 0.5f;
		cc.float32[1] = 0.5f;
		cc.float32[2] = 1.0f;
		cc.float32[3] = 1.0f;
		VkClearValue cv = { cc };
		rpBeginInfo.pClearValues = &cv;

		std::vector<VkImageLayout> newLayouts;
		newLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		newLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

		m_FrameBuffer.ChangeLayout(*cmdBuffer, newLayouts);
		cmdBuffer->PushPipelineBarrier();
		vkCmdBeginRenderPass(cmdBuffer->CmdBuffer(), &rpBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(cmdBuffer->CmdBuffer(), 0, 1, &m_Viewport);
		vkCmdSetScissor(cmdBuffer->CmdBuffer(), 0, 1, &m_Scissor);
		//transfer from fbo to back buffer
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "Tonemapping");
		m_ToneMapping.Render(*cmdBuffer, m_Viewport, VK_FRAME_INDEX);
		//render imgui on top
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "ImGui");
		ImGui_ImplGlfwVulkan_Render(cmdBuffer->CmdBuffer());
		m_Profiler.Stamp(cmdBuffer->CmdBuffer(), "End");
		vkCmdEndRenderPass(cmdBuffer->CmdBuffer());

		m_CmdBufferFactory.EndBuffer(cmdBuffer);

		m_vkQueue.Submit(cmdBuffer->CmdBuffer(), m_RenderCompleteSemaphore, m_ImguiComplete, m_Fence[VK_FRAME_INDEX]);
	}
}

void GraphicsEngine::Swap() {

	vkWaitForFences(VK_DEVICE, 1, &m_Fence[VK_FRAME_INDEX], true, UINT64_MAX);
	vkResetFences(VK_DEVICE, 1, &m_Fence[VK_FRAME_INDEX]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pImageIndices = &VK_FRAME_INDEX;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_VKSwapChain.SwapChain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_ImguiComplete;
	vkQueuePresentKHR(m_vkQueue.GetQueue(), &presentInfo);

	m_RenderQueues[VK_FRAME_INDEX].Clear();

	vkAcquireNextImageKHR(VK_DEVICE, m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr, &VK_FRAME_INDEX);
	m_DeviceAllocator.Clear();
	m_Profiler.Reset(VK_DEVICE);
}

RenderQueue* GraphicsEngine::GetRenderQueue() {
	return &m_RenderQueues[VK_FRAME_INDEX];
}

RenderQueue* GraphicsEngine::GetStaticQueue() {
	return &m_StaticRenderQueue;
}

void GraphicsEngine::TransferToGPU() {
	CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
	cmdBuffer->Begin(nullptr, nullptr);
	m_Resources.ScheduleTransfer(*cmdBuffer);
	m_CmdBufferFactory.EndBuffer(cmdBuffer);
	m_StaticRenderQueue.ScheduleTransfer(m_DeviceAllocator);
	m_vkQueue.Submit(cmdBuffer->CmdBuffer());
	vkQueueWaitIdle(m_vkQueue.GetQueue());
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
		CommandBuffer* cmdBuffer = m_CmdBufferFactory.GetNextBuffer();
		cmdBuffer->Begin(nullptr, nullptr);

		ImGui_ImplGlfwVulkan_CreateFontsTexture(cmdBuffer->CmdBuffer());
		
		m_CmdBufferFactory.EndBuffer(cmdBuffer);
		m_vkQueue.Submit(cmdBuffer->CmdBuffer());
		
		//vkQueueWaitIdle(m_vkQueue.GetQueue());
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