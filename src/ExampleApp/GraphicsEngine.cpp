#include "GraphicsEngine.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#ifdef _WIN32
#include <Windows.h>
#endif
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>
#include "VulkanContext.h"
#include "Vertex.h"

GraphicsEngine::GraphicsEngine() {

}
GraphicsEngine::~GraphicsEngine() {
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
	VkDebugReportFlagsEXT       flags,
	VkDebugReportObjectTypeEXT  objectType,
	uint64_t                    object,
	size_t                      location,
	int32_t                     messageCode,
	const char*                 pLayerPrefix,
	const char*                 pMessage,
	void*                       pUserData)
{

#ifdef _WIN32
	wchar_t msg[1024];
	mbstowcs(msg, pMessage, 1024);
	OutputDebugString(msg);
	OutputDebugString(L"\n");
#else
	printf("%s\n", pMessage);
#endif
	return VK_FALSE;
}

void GraphicsEngine::Init(GLFWwindow* window) {
	m_CurrentPipeline = 0;

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "Tephra";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "Tephra";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	uint32_t instExtCount = 0;
	const char** instExt = glfwGetRequiredInstanceExtensions(&instExtCount);
	std::vector<const char*> extensions;

	for (uint32_t i = 0; i < instExtCount; i++) {
		extensions.push_back(instExt[i]);
	}
#ifdef _DEBUG
	extensions.push_back("VK_EXT_debug_report");
#endif
	std::vector<const char*> layers;
#ifdef _DEBUG
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	vk::InstanceCreateInfo instInfo;
	instInfo.enabledExtensionCount = extensions.size();
	instInfo.pApplicationInfo = &appInfo;
	instInfo.ppEnabledExtensionNames = &extensions[0];
	instInfo.enabledLayerCount = layers.size();
	instInfo.ppEnabledLayerNames = &layers[0];

	m_VKContext.Instance = vk::createInstance(instInfo);
#ifdef _DEBUG
	/* Load VK_EXT_debug_report entry points in debug builds */
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>
		(m_VKContext.Instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
	PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
		reinterpret_cast<PFN_vkDebugReportMessageEXT>
		(m_VKContext.Instance.getProcAddr("vkDebugReportMessageEXT"));
	PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>
		(m_VKContext.Instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));

	VkDebugReportCallbackCreateInfoEXT debugCallbacks;
	debugCallbacks.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugCallbacks.pfnCallback = &VulkanDebugCallback;

	//m_DebugCallbacks = m_VKContext.Instance.createDebugReportCallbackEXT( debugCallbacks);
	vkCreateDebugReportCallbackEXT(m_VKContext.Instance, &debugCallbacks, nullptr, &m_DebugCallbacks);
#endif

	auto gpus = m_VKContext.Instance.enumeratePhysicalDevices();
	if (gpus.size() == 0) {
		return;
	}
	//grab first available GPU
	for(auto& gpu : gpus){
		bool lp = gpu.getFeatures().largePoints;
		if (gpu.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu ||
			gpu.getProperties().deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
			m_VKContext.PhysicalDevice = gpu;
			break;
		}
	}

	int queueFamilyIndex = 0;
	for (auto& queue : m_VKContext.PhysicalDevice.getQueueFamilyProperties()) {
		if (queue.queueFlags & vk::QueueFlagBits::eGraphics) {
			break;
		}
		queueFamilyIndex++;
	}
	float queuePrio[] = { 0.0f };
	vk::DeviceQueueCreateInfo queueInfo;
	queueInfo.queueCount = 1;
	queueInfo.queueFamilyIndex = queueFamilyIndex;
	queueInfo.pQueuePriorities = queuePrio;

	std::vector<std::string> deviceExtensionStrings;
	for (auto& extension : m_VKContext.PhysicalDevice.enumerateDeviceExtensionProperties()) {
		deviceExtensionStrings.push_back(extension.extensionName);
	}
	std::vector<std::string> devicelayerStrings;
	for (auto& layer : m_VKContext.PhysicalDevice.enumerateDeviceLayerProperties()) {
		devicelayerStrings.push_back(layer.layerName);
	}
	std::vector<const char*> devicelayers;
	std::vector<const char*> deviceExtensions;

	for (auto& ext : deviceExtensionStrings) {
		deviceExtensions.push_back(ext.c_str());
	}
	for (auto& layer : devicelayerStrings) {
		devicelayers.push_back(layer.c_str());
	}

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = &deviceExtensions[0];
	deviceInfo.enabledLayerCount = devicelayers.size();
	deviceInfo.ppEnabledLayerNames = &devicelayers[0];
	deviceInfo.pEnabledFeatures = nullptr;

	m_VKContext.Device = m_VKContext.PhysicalDevice.createDevice(deviceInfo);
	//get queues
	m_VKContext.GFXQueue = m_VKContext.Device.getQueue(queueFamilyIndex, 0);

	//set up command buffer
	vk::CommandPoolCreateInfo poolInfo;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	m_CmdBuffer.CmdPool[0] = m_VKContext.Device.createCommandPool(poolInfo);
	m_CmdBuffer.CmdPool[1] = m_VKContext.Device.createCommandPool(poolInfo);

	vk::CommandBufferAllocateInfo bufferInfo;
	bufferInfo.commandBufferCount = 1;
	bufferInfo.commandPool = m_CmdBuffer.CmdPool[0];
	bufferInfo.level = vk::CommandBufferLevel::ePrimary;
	m_CmdBuffer.CmdBuffer = m_VKContext.Device.allocateCommandBuffers(bufferInfo)[0];

	//surface and swapchain
	VkSurfaceKHR surf;
	glfwCreateWindowSurface(m_VKContext.Instance, window, nullptr, &surf);
	m_VKSwapChain.Surface = surf;

	m_VKContext.PhysicalDevice.getSurfaceSupportKHR(queueFamilyIndex, m_VKSwapChain.Surface);
	auto formats = m_VKContext.PhysicalDevice.getSurfaceFormatsKHR(m_VKSwapChain.Surface);
	auto surfCapabilities = m_VKContext.PhysicalDevice.getSurfaceCapabilitiesKHR(m_VKSwapChain.Surface);
	auto presentModes = m_VKContext.PhysicalDevice.getSurfacePresentModesKHR(m_VKSwapChain.Surface);

	vk::SurfaceFormatKHR format = formats[0];
	//find out if we support srgb format
	for (auto& f : formats) {
		if (f.format == vk::Format::eB8G8R8A8Srgb)
			format = f;
	}
	bool vsync = true;
	vk::PresentModeKHR mode = vsync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;

	vk::SwapchainCreateInfoKHR swapChainInfo;
	swapChainInfo.clipped = true;
	swapChainInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapChainInfo.surface = m_VKSwapChain.Surface;
	swapChainInfo.imageFormat = format.format;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageColorSpace = format.colorSpace;
	swapChainInfo.imageExtent = surfCapabilities.currentExtent;
	swapChainInfo.imageSharingMode = vk::SharingMode::eExclusive;
	swapChainInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	swapChainInfo.minImageCount = surfCapabilities.minImageCount;
	swapChainInfo.presentMode = mode;
	swapChainInfo.preTransform = surfCapabilities.currentTransform;
	swapChainInfo.queueFamilyIndexCount = 0;
	swapChainInfo.pQueueFamilyIndices = nullptr;

	m_VKSwapChain.SwapChain = m_VKContext.Device.createSwapchainKHR(swapChainInfo);
	//get images from swapchain
	uint32_t imageCount;
	m_VKContext.Device.getSwapchainImagesKHR(m_VKSwapChain.SwapChain, &imageCount, m_VKSwapChain.Images);

	for (int i = 0; i < BUFFER_COUNT; i++) {
		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.components = { vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		imageViewInfo.format = format.format;
		imageViewInfo.image = m_VKSwapChain.Images[i];
		imageViewInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
		m_VKSwapChain.ImageViews[i] = m_VKContext.Device.createImageView(imageViewInfo);
	}
	//renderpass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.dependencyCount = 0;
	vk::AttachmentDescription attachmentDesc;
	attachmentDesc.format = format.format;
	attachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDesc.samples = vk::SampleCountFlagBits::e1;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &attachmentDesc;
	vk::SubpassDescription subPassDesc;
	subPassDesc.colorAttachmentCount = 1;
	subPassDesc.inputAttachmentCount = 0;
	subPassDesc.pColorAttachments = &vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
	subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subPassDesc.pDepthStencilAttachment = nullptr;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.pPreserveAttachments = nullptr;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;
	m_RenderPass = m_VKContext.Device.createRenderPass(renderPassInfo);

	//framebuffers
	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.height = surfCapabilities.currentExtent.height;
	framebufferInfo.width = surfCapabilities.currentExtent.width;
	framebufferInfo.layers = 1;
	framebufferInfo.pAttachments = &m_VKSwapChain.ImageViews[0];
	framebufferInfo.renderPass = m_RenderPass;

	m_VKSwapChain.FrameBuffers[0] = m_VKContext.Device.createFramebuffer(framebufferInfo);
	framebufferInfo.pAttachments = &m_VKSwapChain.ImageViews[1];
	m_VKSwapChain.FrameBuffers[1] = m_VKContext.Device.createFramebuffer(framebufferInfo);

	vk::SemaphoreCreateInfo semaphoreInfo;
	m_ImageAquiredSemaphore = m_VKContext.Device.createSemaphore(semaphoreInfo);
	m_RenderCompleteSemaphore = m_VKContext.Device.createSemaphore(semaphoreInfo);
	vk::FenceCreateInfo fenceInfo;
	m_Fence[0] = m_VKContext.Device.createFence(fenceInfo);
	m_Fence[1] = m_VKContext.Device.createFence(fenceInfo);

	m_VKContext.FrameIndex = m_VKContext.Device.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;
	//viewport
	int w, h;
	glfwGetWindowSize(window, &w, &h);
	m_Viewport.width = w;
	m_Viewport.height = h;
	m_Viewport.minDepth = 0.0f;
	m_Viewport.maxDepth = 1.0f;
	m_Viewport.x = 0;
	m_Viewport.y = 0;
	//load pipelines
	m_Pipelines[0].SetDefaultVertexState(Vertex::GetVertexState());
	m_Pipelines[1].SetDefaultVertexState(Vertex::GetVertexState());
	m_Pipelines[2].SetDefaultVertexState(Vertex::GetVertexState());
	m_Pipelines[0].LoadPipelineFromFile(m_VKContext.Device, "shader/filled.json", m_Viewport, m_RenderPass);
	m_Pipelines[1].LoadPipelineFromFile(m_VKContext.Device, "shader/wireframe.json", m_Viewport, m_RenderPass);
	m_Pipelines[2].LoadPipelineFromFile(m_VKContext.Device, "shader/frontface.json", m_Viewport, m_RenderPass);
	//create mesh
	 par_shapes_mesh_s* mesh = par_shapes_create_icosahedron();
	 //unweld to create per vertex normals
	 par_shapes_unweld(mesh, true);
	 par_shapes_compute_normals(mesh);
	std::vector<Vertex::Vertex> vertices;
	Vertex::Vertex v;
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		v.PosL = glm::vec3(mesh->points[mesh->triangles[i] * 3], mesh->points[mesh->triangles[i] * 3 + 1], mesh->points[mesh->triangles[i] * 3 + 2]);
		v.Normal = glm::vec3(mesh->normals[mesh->triangles[i] * 3], mesh->normals[mesh->triangles[i] * 3 + 1], mesh->normals[mesh->triangles[i] * 3 + 2]);
		vertices.push_back(v);
		v.PosL = glm::vec3(mesh->points[mesh->triangles[i + 1] * 3], mesh->points[mesh->triangles[i + 1] * 3 + 1], mesh->points[mesh->triangles[i + 1] * 3 + 2]);
		v.Normal = glm::vec3(mesh->normals[mesh->triangles[i + 1] * 3], mesh->normals[mesh->triangles[i + 1] * 3 + 1], mesh->normals[mesh->triangles[i + 1] * 3 + 2]);
		vertices.push_back(v);
		v.PosL = glm::vec3(mesh->points[mesh->triangles[i + 2] * 3], mesh->points[mesh->triangles[i + 2] * 3 + 1], mesh->points[mesh->triangles[i + 2] * 3 + 2]);
		v.Normal = glm::vec3(mesh->normals[mesh->triangles[i + 2] * 3], mesh->normals[mesh->triangles[i + 2] * 3 + 1], mesh->normals[mesh->triangles[i + 2] * 3 + 2]);
		vertices.push_back(v);
	}
	m_MeshVertexCount = vertices.size();
	//create vertex buffer
	vk::BufferCreateInfo vertexBufferInfo;
	vertexBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	vertexBufferInfo.size = sizeof(Vertex::Vertex) * vertices.size();
	vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
	m_VertexBuffer = m_VKContext.Device.createBuffer(vertexBufferInfo);
	vk::MemoryRequirements memReq = m_VKContext.Device.getBufferMemoryRequirements(m_VertexBuffer);
	//find the correct heap
	vk::PhysicalDeviceMemoryProperties devMemProp = m_VKContext.PhysicalDevice.getMemoryProperties();
	for (uint32_t i = 0; i < devMemProp.memoryTypeCount; i++) {
		if (memReq.memoryTypeBits & (1 << i) &&
			devMemProp.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) {
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = i;
			m_VertexMemory = m_VKContext.Device.allocateMemory(allocInfo);
			break;
		}
	}
	//bind buffer to memory
	m_VKContext.Device.bindBufferMemory(m_VertexBuffer, m_VertexMemory, 0);
	//transfer data
	void* data = m_VKContext.Device.mapMemory(m_VertexMemory, 0, VK_WHOLE_SIZE);
	memcpy(data, vertices.data(), sizeof(Vertex::Vertex) * vertices.size());
	m_VKContext.Device.unmapMemory(m_VertexMemory);
	par_shapes_free_mesh(mesh);

	vk::DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.maxSets = 1;
	vk::DescriptorPoolSize poolSize;
	poolSize.descriptorCount = 1;
	poolSize.type = vk::DescriptorType::eUniformBuffer;
	descPoolInfo.pPoolSizes = &poolSize;
	descPoolInfo.poolSizeCount = 1;
	m_DescriptorPool = m_VKContext.Device.createDescriptorPool(descPoolInfo);

	vk::DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.descriptorPool = m_DescriptorPool;
	descSetAllocInfo.descriptorSetCount = 1;
	descSetAllocInfo.pSetLayouts = m_Pipelines[m_CurrentPipeline].GetDescriptorSetLayouts().data();
	m_VKContext.Device.allocateDescriptorSets(&descSetAllocInfo, &m_DescriptorSet);
	//create vertex buffer
	vk::BufferCreateInfo uniformBufferInfo;
	uniformBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	uniformBufferInfo.size = sizeof(glm::mat4);
	uniformBufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
	m_UniformBuffer = m_VKContext.Device.createBuffer(uniformBufferInfo);
	memReq = m_VKContext.Device.getBufferMemoryRequirements(m_UniformBuffer);
	//find the correct heap
	for (uint32_t i = 0; i < devMemProp.memoryTypeCount; i++) {
		if (memReq.memoryTypeBits & (1 << i) &&
			devMemProp.memoryTypes[i].propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible) {
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = memReq.size;
			allocInfo.memoryTypeIndex = i;
			m_UniformMemory = m_VKContext.Device.allocateMemory(allocInfo);
			break;
		}
	}
	//bind buffer to memory
	m_VKContext.Device.bindBufferMemory(m_UniformBuffer, m_UniformMemory, 0);
	//transfer data

	glm::mat4 camera = glm::perspectiveFov(1.5f, 1600.0f, 900.0f, 0.1f, 100.0f) * glm::lookAt(glm::vec3(0, 0.5f, -2.5f), glm::vec3(0), glm::vec3(0, 1, 0));
	data = m_VKContext.Device.mapMemory(m_UniformMemory, 0, VK_WHOLE_SIZE);
	memcpy(data, &camera, sizeof(glm::mat4));
	m_VKContext.Device.unmapMemory(m_UniformMemory);

	vk::WriteDescriptorSet descWrite;
	descWrite.descriptorCount = 1;
	descWrite.descriptorType = vk::DescriptorType::eUniformBuffer;
	descWrite.dstArrayElement = 0;
	descWrite.dstBinding = 1;
	descWrite.dstSet = m_DescriptorSet;
	vk::DescriptorBufferInfo descBufferInfo;
	descBufferInfo.buffer = m_UniformBuffer;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	descWrite.pBufferInfo = &descBufferInfo;
	m_VKContext.Device.updateDescriptorSets(1, &descWrite, 0, nullptr);
}

void GraphicsEngine::Render() {
	m_VKContext.Device.resetCommandPool(m_CmdBuffer.CmdPool[m_VKContext.FrameIndex], vk::CommandPoolResetFlagBits::eReleaseResources);

	vk::CommandBufferBeginInfo beginInfo;
	vk::CommandBufferInheritanceInfo inheritInfo;
	inheritInfo.framebuffer = m_VKSwapChain.FrameBuffers[m_VKContext.FrameIndex];
	inheritInfo.renderPass = m_RenderPass;
	beginInfo.pInheritanceInfo = &inheritInfo;

	m_CmdBuffer.CmdBuffer.begin(beginInfo);

	//set layout to color attachment for current backbuffer
	vk::ImageMemoryBarrier imgMemBarrier = {};
	imgMemBarrier.image = m_VKSwapChain.Images[m_VKContext.FrameIndex];
	imgMemBarrier.oldLayout = vk::ImageLayout::ePresentSrcKHR;
	imgMemBarrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	imgMemBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	imgMemBarrier.subresourceRange.baseMipLevel = 0;
	imgMemBarrier.subresourceRange.baseArrayLayer = 0;
	imgMemBarrier.subresourceRange.layerCount = 1;
	imgMemBarrier.subresourceRange.levelCount = 1;
	m_CmdBuffer.CmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);

	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.framebuffer = m_VKSwapChain.FrameBuffers[m_VKContext.FrameIndex];
	renderPassInfo.renderPass = m_RenderPass;
	glm::vec4 color = glm::vec4(100.0f / 255, 149.0f / 255, 237.0f / 255,1);
	vk::ClearColorValue clearColor;
	clearColor.float32[0] = color.r;
	clearColor.float32[1] = color.g;
	clearColor.float32[2] = color.b;
	clearColor.float32[3] = 1;
	vk::ClearValue clearValues[1] = { clearColor };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = clearValues;
	renderPassInfo.renderArea = { 0, 0, 1600, 900 };

	m_CmdBuffer.CmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	//render here
	vk::DeviceSize offset = 0;
	m_CmdBuffer.CmdBuffer.bindVertexBuffers(0, 1, &m_VertexBuffer, &offset);
	m_CmdBuffer.CmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[m_CurrentPipeline].GetPipeline());
	m_CmdBuffer.CmdBuffer.setViewport(0, 1, &m_Viewport);
	m_CmdBuffer.CmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[m_CurrentPipeline].GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
	m_CmdBuffer.CmdBuffer.draw(m_MeshVertexCount, 1, 0, 0);

	m_CmdBuffer.CmdBuffer.endRenderPass();

	imgMemBarrier.oldLayout = vk::ImageLayout::eColorAttachmentOptimal;
	imgMemBarrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
	imgMemBarrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
	imgMemBarrier.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
	m_CmdBuffer.CmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTopOfPipe,
		vk::DependencyFlagBits::eByRegion, 0, nullptr, 0, nullptr, 1, &imgMemBarrier);

	m_CmdBuffer.CmdBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_CmdBuffer.CmdBuffer;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_ImageAquiredSemaphore;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_RenderCompleteSemaphore;
	vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eBottomOfPipe;
	submitInfo.pWaitDstStageMask = &flags;

	m_VKContext.GFXQueue.submit(submitInfo, m_Fence[m_VKContext.FrameIndex]);
}

void GraphicsEngine::Swap() {

	m_VKContext.Device.waitForFences(1, &m_Fence[m_VKContext.FrameIndex], true, UINT64_MAX);
	m_VKContext.Device.resetFences(1, &m_Fence[m_VKContext.FrameIndex]);

	vk::PresentInfoKHR presentInfo;
	presentInfo.pImageIndices = &m_VKContext.FrameIndex;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_VKSwapChain.SwapChain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RenderCompleteSemaphore;
	m_VKContext.GFXQueue.presentKHR(presentInfo);

	m_VKContext.FrameIndex = m_VKContext.Device.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;
}

void GraphicsEngine::NextPipeline() {
	m_CurrentPipeline = (++m_CurrentPipeline) % 3;
}

void GraphicsEngine::PrevPipeline() {
	m_CurrentPipeline = (m_CurrentPipeline + 2) % 3;
}
