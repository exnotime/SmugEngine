#include "GraphicsEngine.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>
#include "Vertex.h"
#include <vulkan/vulkan.h>

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
	if (messageCode == 3 || messageCode == 15) {
		return VK_FALSE;
	}

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

void GraphicsEngine::CreateContext() {

	vk::ApplicationInfo appInfo;
	appInfo.pApplicationName = "Tephra";
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "Tephra";
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	std::vector<const char*> layers;
	std::vector<const char*> extensions;
#ifdef _DEBUG
	extensions.push_back("VK_EXT_debug_report");

	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	extensions.push_back("VK_KHR_surface");
	//extensions.push_back("VK_KHR_swapchain");
#ifdef _WIN32
	extensions.push_back("VK_KHR_win32_surface");
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

	vkCreateDebugReportCallbackEXT(m_VKContext.Instance, &debugCallbacks, nullptr, &m_DebugCallbacks);
#endif
	auto gpus = m_VKContext.Instance.enumeratePhysicalDevices();
	if (gpus.size() == 0) {
		return;
	}
	//grab first available GPU
	for (auto& gpu : gpus) {
		bool lp = gpu.getFeatures().largePoints;
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

	for (auto& ext : deviceExtensionStrings) {
		deviceExtensions.push_back(ext.c_str());
	}
	for (auto& layer : devicelayerStrings) {
		devicelayers.push_back(layer.c_str());
	}

	vk::DeviceCreateInfo deviceInfo;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &m_vkQueue.GetInfo();
	deviceInfo.enabledExtensionCount = deviceExtensions.size();
	deviceInfo.ppEnabledExtensionNames = &deviceExtensions[0];
	deviceInfo.enabledLayerCount = devicelayers.size();
	deviceInfo.ppEnabledLayerNames = &devicelayers[0];
	deviceInfo.pEnabledFeatures = nullptr;
	VK_DEVICE = VK_PHYS_DEVICE.createDevice(deviceInfo);
	//get queues
	*static_cast<vk::Queue*>(&m_vkQueue) = VK_DEVICE.getQueue(m_vkQueue.GetQueueIndex(), 0);

	//set up command buffer
	m_vkCmdBuffer.Init(VK_DEVICE, m_vkQueue.GetQueueIndex());
	m_vkMarchCmdBuffer.Init(VK_DEVICE, m_vkQueue.GetQueueIndex());
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
		if (f.format == vk::Format::eB8G8R8A8Srgb)
			format = f;
	}

	vk::PresentModeKHR mode = m_VSync ? vk::PresentModeKHR::eFifo : vk::PresentModeKHR::eMailbox;
	vk::SampleCountFlagBits msaaCount = m_MSAA ? vk::SampleCountFlagBits::e4 : vk::SampleCountFlagBits::e1;

	m_VKSwapChain.MSAA = m_MSAA;
	m_VKSwapChain.Format = format.format;
	m_VKSwapChain.SampleCount = msaaCount;

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
	if (m_MSAA) {
		VK_DEVICE.getSwapchainImagesKHR(m_VKSwapChain.SwapChain, &swapChainImageCount, m_VKSwapChain.ResolveImages);
	} else {
		VK_DEVICE.getSwapchainImagesKHR(m_VKSwapChain.SwapChain, &swapChainImageCount, m_VKSwapChain.Images);
	}

	for (int i = 0; i < BUFFER_COUNT; i++) {
		if (m_MSAA) {
			vk::ImageCreateInfo imageInfo;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = vk::Extent3D(surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height, 1);
			imageInfo.format = format.format;
			imageInfo.imageType = vk::ImageType::e2D;
			imageInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageInfo.mipLevels = 1;
			imageInfo.samples = msaaCount;
			imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
			m_VKSwapChain.Images[i] = VK_DEVICE.createImage(imageInfo);

			m_TextureMemory.AllocateImage(m_VKSwapChain.Images[i]);
		}

		vk::ImageViewCreateInfo imageViewInfo;
		imageViewInfo.components = { vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		imageViewInfo.format = format.format;
		imageViewInfo.image = m_VKSwapChain.Images[i];
		imageViewInfo.viewType = vk::ImageViewType::e2D;
		imageViewInfo.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 };
		m_VKSwapChain.ImageViews[i] = VK_DEVICE.createImageView(imageViewInfo);
	}
	//create depth stencil
	for (int i = 0; i < BUFFER_COUNT; i++) {
		vk::ImageCreateInfo dsiInfo;
		dsiInfo.arrayLayers = 1;
		dsiInfo.extent = vk::Extent3D(surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height, 1);
		dsiInfo.format = vk::Format::eD24UnormS8Uint;
		dsiInfo.imageType = vk::ImageType::e2D;
		dsiInfo.initialLayout = vk::ImageLayout::eUndefined;
		dsiInfo.mipLevels = 1;
		dsiInfo.samples = msaaCount;
		dsiInfo.tiling = vk::ImageTiling::eOptimal;
		dsiInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
		m_VKSwapChain.DepthStencilImages[i] = VK_DEVICE.createImage(dsiInfo);

		if (m_VKSwapChain.MSAA) {
			dsiInfo.arrayLayers = 1;
			dsiInfo.extent = vk::Extent3D(surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height, 1);
			dsiInfo.format = vk::Format::eD24UnormS8Uint;
			dsiInfo.imageType = vk::ImageType::e2D;
			dsiInfo.initialLayout = vk::ImageLayout::eUndefined;
			dsiInfo.mipLevels = 1;
			dsiInfo.samples = vk::SampleCountFlagBits::e1;
			dsiInfo.tiling = vk::ImageTiling::eOptimal;
			dsiInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
			m_VKSwapChain.DepthResolveImages[i] = VK_DEVICE.createImage(dsiInfo);
			m_TextureMemory.AllocateImage(m_VKSwapChain.DepthResolveImages[i]);
		}

		//bind memory
		m_TextureMemory.AllocateImage(m_VKSwapChain.DepthStencilImages[i]);
		//create View
		vk::ImageViewCreateInfo dsiViewInfo;
		dsiViewInfo.components = { vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
		dsiViewInfo.format = dsiInfo.format;
		dsiViewInfo.image = m_VKSwapChain.DepthStencilImages[i];
		dsiViewInfo.viewType = vk::ImageViewType::e2D;
		dsiViewInfo.subresourceRange = { vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1 };
		m_VKSwapChain.DepthStencilImageViews[i] = VK_DEVICE.createImageView(dsiViewInfo);

	}

	//renderpass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.dependencyCount = 0;
	std::vector<vk::AttachmentDescription> attachments;
	vk::AttachmentDescription attachmentDesc;
	attachmentDesc.format = format.format;
	attachmentDesc.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDesc.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDesc.samples = msaaCount;
	attachments.push_back(attachmentDesc);

	attachmentDesc = {};
	attachmentDesc.format = vk::Format::eD24UnormS8Uint;
	attachmentDesc.initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachmentDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
	attachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
	attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
	attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	attachmentDesc.samples = msaaCount;
	attachments.push_back(attachmentDesc);

	renderPassInfo.attachmentCount = attachments.size();
	renderPassInfo.pAttachments = attachments.data();

	vk::SubpassDescription subPassDesc;
	subPassDesc.colorAttachmentCount = 1;
	subPassDesc.inputAttachmentCount = 0;
	vk::AttachmentReference colorRef = { 0, vk::ImageLayout::eColorAttachmentOptimal };
	subPassDesc.pColorAttachments = &colorRef;
	subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	vk::AttachmentReference depthRef = { 1, vk::ImageLayout::eDepthStencilAttachmentOptimal };
	subPassDesc.pDepthStencilAttachment = &depthRef;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.pPreserveAttachments = nullptr;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;
	m_RenderPass = VK_DEVICE.createRenderPass(renderPassInfo);

	//framebuffers
	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.attachmentCount = 2;
	framebufferInfo.height = surfCapabilities.currentExtent.height;
	framebufferInfo.width = surfCapabilities.currentExtent.width;
	framebufferInfo.layers = 1;
	vk::ImageView fbViews1[] = { m_VKSwapChain.ImageViews[0], m_VKSwapChain.DepthStencilImageViews[0] };
	framebufferInfo.pAttachments = fbViews1;
	framebufferInfo.renderPass = m_RenderPass;
	m_VKSwapChain.FrameBuffers[0] = VK_DEVICE.createFramebuffer(framebufferInfo);

	vk::ImageView fbViews2[] = { m_VKSwapChain.ImageViews[1], m_VKSwapChain.DepthStencilImageViews[1] };
	framebufferInfo.pAttachments = fbViews2;
	m_VKSwapChain.FrameBuffers[1] = VK_DEVICE.createFramebuffer(framebufferInfo);

	if (m_MSAA) {
		m_MSState.sampleShadingEnable = true;
		m_MSState.rasterizationSamples = msaaCount;
	}
}

void GraphicsEngine::Init(glm::vec2 windowSize, bool vsync, HWND hWnd) {
	m_CurrentPipeline = 0;
	CreateContext();

	vk::Win32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.hwnd = hWnd;
	surfaceInfo.hinstance = GetModuleHandle(nullptr);

	m_VKSwapChain.Surface = m_VKContext.Instance.createWin32SurfaceKHR(surfaceInfo);

	//Allocate memory on gpu
	m_TextureMemory.Init(VK_DEVICE, VK_PHYS_DEVICE, 512 * MEGA_BYTE);
	m_BufferMemory.Init(VK_DEVICE, VK_PHYS_DEVICE, 16 * MEGA_BYTE, 8 * MEGA_BYTE);

	m_MSAA = false;
	m_VSync = vsync;
	m_ScreenSize = windowSize;
	CreateSwapChain(m_VKSwapChain.Surface);

	vk::SemaphoreCreateInfo semaphoreInfo;
	m_ImageAquiredSemaphore = VK_DEVICE.createSemaphore(semaphoreInfo);
	m_RenderCompleteSemaphore = VK_DEVICE.createSemaphore(semaphoreInfo);
	m_RayMarchComplete = VK_DEVICE.createSemaphore(semaphoreInfo);
	vk::FenceCreateInfo fenceInfo;
	m_Fence[0] = VK_DEVICE.createFence(fenceInfo);
	m_Fence[1] = VK_DEVICE.createFence(fenceInfo);

	VK_FRAME_INDEX = VK_DEVICE.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;
	//viewport
	m_Viewport.width = m_ScreenSize.x;
	m_Viewport.height = m_ScreenSize.y;
	m_Viewport.minDepth = 0.0f;
	m_Viewport.maxDepth = 1.0f;
	m_Viewport.x = 0;
	m_Viewport.y = 0;
	//load pipelines
	m_Pipelines[0].SetDefaultVertexState(Geometry::GetVertexState());
	m_Pipelines[1].SetDefaultVertexState(Geometry::GetVertexState());
	m_Pipelines[2].SetDefaultVertexState(Geometry::GetVertexState());

	if (m_MSAA) {
		m_Pipelines[0].SetDefaultMulitSampleState(m_MSState);
		m_Pipelines[1].SetDefaultMulitSampleState(m_MSState);
		m_Pipelines[2].SetDefaultMulitSampleState(m_MSState);
	}

	m_Pipelines[0].LoadPipelineFromFile(VK_DEVICE, "shader/filled.json", m_Viewport, m_RenderPass);
	m_Pipelines[1].LoadPipelineFromFile(VK_DEVICE, "shader/wireframe.json", m_Viewport, m_RenderPass);
	m_Pipelines[2].LoadPipelineFromFile(VK_DEVICE, "shader/frontface.json", m_Viewport, m_RenderPass);
	//create mesh
	par_shapes_mesh_s* mesh = par_shapes_create_subdivided_sphere(5);

	std::vector<Geometry::Vertex> vertices;
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		for (int k = 0; k < 3; k++) {
			Geometry::Vertex v;
			v.PosL = glm::vec3(mesh->points[mesh->triangles[i + k] * 3], mesh->points[mesh->triangles[i + k] * 3 + 1], mesh->points[mesh->triangles[i + k] * 3 + 2]);
			v.Normal = glm::vec3(mesh->normals[mesh->triangles[i + k] * 3], mesh->normals[mesh->triangles[i + k] * 3 + 1], mesh->normals[mesh->triangles[i + k] * 3 + 2]);
			if (!mesh->tcoords) {
				if (v.Normal == glm::vec3(0, 0, -1) || v.Normal == glm::vec3(0, 0, 1)) {
					v.TexCoord.x = v.PosL.x * 0.5f + 0.5f;
					v.TexCoord.y = v.PosL.y * 0.5f + 0.5f;
				}
				else if (v.Normal == glm::vec3(0, -1, 0) || v.Normal == glm::vec3(0, 1, 0)) {
					v.TexCoord.x = v.PosL.x * 0.5f + 0.5f;
					v.TexCoord.y = v.PosL.z * 0.5f + 0.5f;
				}
				else if (v.Normal == glm::vec3(-1, 0, 0) || v.Normal == glm::vec3(1, 0, 0)) {
					v.TexCoord.x = v.PosL.z * 0.5f + 0.5f;
					v.TexCoord.y = v.PosL.y * 0.5f + 0.5f;
				}
			} else {
				v.TexCoord = glm::vec2(mesh->tcoords[mesh->triangles[i + k] * 3], mesh->tcoords[mesh->triangles[i + k] * 3 + 1]);
			}
			vertices.push_back(v);
		}
	}
	m_MeshVertexCount = vertices.size();

	//create vertex buffer
	m_VertexBuffer = m_BufferMemory.AllocateBuffer(sizeof(Geometry::Vertex) * vertices.size(), vk::BufferUsageFlagBits::eVertexBuffer, vertices.data());
	par_shapes_free_mesh(mesh);

	m_UniformBuffer = m_BufferMemory.AllocateBuffer(sizeof(PerFrameBuffer), vk::BufferUsageFlagBits::eUniformBuffer, nullptr);

	m_Texture.Init("assets/skybox.dds", m_TextureMemory, VK_DEVICE);

	vk::DescriptorPoolCreateInfo descPoolInfo;
	descPoolInfo.maxSets = 1;
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1));
	poolSizes.push_back(vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1));
	descPoolInfo.pPoolSizes = poolSizes.data();
	descPoolInfo.poolSizeCount = poolSizes.size();
	m_DescriptorPool = VK_DEVICE.createDescriptorPool(descPoolInfo);

	vk::DescriptorSetAllocateInfo descSetAllocInfo;
	descSetAllocInfo.descriptorPool = m_DescriptorPool;
	descSetAllocInfo.descriptorSetCount = 1;
	descSetAllocInfo.pSetLayouts = m_Pipelines[m_CurrentPipeline].GetDescriptorSetLayouts().data();
	VK_DEVICE.allocateDescriptorSets(&descSetAllocInfo, &m_DescriptorSet);

	vk::WriteDescriptorSet descWrites[2];
	descWrites[0].descriptorCount = 1;
	descWrites[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	descWrites[0].dstArrayElement = 0;
	descWrites[0].dstBinding = 0;
	descWrites[0].dstSet = m_DescriptorSet;
	vk::DescriptorBufferInfo descBufferInfo;
	descBufferInfo.buffer = m_UniformBuffer.BufferHandle;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	descWrites[0].pBufferInfo = &descBufferInfo;

	descWrites[1].descriptorCount = 1;
	descWrites[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	descWrites[1].dstArrayElement = 0;
	descWrites[1].dstBinding = 1;
	descWrites[1].dstSet = m_DescriptorSet;
	vk::DescriptorImageInfo imgInfo = m_Texture.GetDescriptorInfo();
	descWrites[1].pImageInfo = &imgInfo;

	VK_DEVICE.updateDescriptorSets(2, descWrites, 0, nullptr);

	m_SkyBox.Init(VK_DEVICE, VK_PHYS_DEVICE, "assets/skybox.dds", m_Viewport, m_RenderPass, m_MSState);
	m_Raymarcher.Init(VK_DEVICE, m_VKSwapChain, VK_PHYS_DEVICE);

	MemoryBudget memBudget;
	memBudget.GeometryBudget = 256 * MEGA_BYTE;
	memBudget.MaterialBudget = 512 * MEGA_BYTE;
	m_Resources.Init(VK_DEVICE, VK_PHYS_DEVICE, memBudget);
	//prepare initial transfer
	m_vkCmdBuffer.Begin(nullptr, nullptr);
	//m_SkyBox.PrepareUniformBuffer(m_vkCmdBuffer, CameraData);
	
	for (int i = 0; i < BUFFER_COUNT; i++) {
		if (m_MSAA) {
			m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.Images[i], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
			m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.ResolveImages[i], vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);

			m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.DepthStencilImages[i], vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		} else {
			m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.DepthStencilImages[i], vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
			m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.Images[i], vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
		}
	}
	m_vkCmdBuffer.PushPipelineBarrier();

	m_BufferMemory.ScheduleTransfers(m_vkCmdBuffer);
	m_TextureMemory.ScheduleTransfers(m_vkCmdBuffer);
	m_vkCmdBuffer.end();

	m_vkQueue.Submit(m_vkCmdBuffer);
	VK_DEVICE.waitIdle();
}

void GraphicsEngine::Render() {
	//transfer new camera data
	RenderQueue& rq = m_RenderQueues[VK_FRAME_INDEX];
	CameraData cd = rq.GetCameras()[0];

	PerFrameBuffer uniformBuffer;
	uniformBuffer.ViewProj = (cd.ProjView * (glm::translate(glm::vec3(0,10,0)) * glm::scale(glm::vec3(2.0f))));
	uniformBuffer.CameraPos = glm::vec4(cd.Position, 1);
	uniformBuffer.LightDir = glm::normalize(glm::vec4(0.2f, -1.0f, -0.4f, 1.0f));
	m_BufferMemory.UpdateBuffer(m_UniformBuffer, sizeof(PerFrameBuffer), (void*)(&uniformBuffer));
	m_vkCmdBuffer.Reset(VK_DEVICE, VK_FRAME_INDEX);

	m_vkCmdBuffer.Begin(nullptr, nullptr);
	m_BufferMemory.ScheduleTransfers(m_vkCmdBuffer);
	m_SkyBox.PrepareUniformBuffer(m_vkCmdBuffer, cd.ProjView, glm::translate(cd.Position));
	m_Raymarcher.UpdateUniforms(m_vkCmdBuffer, cd.ProjView, cd.Position);
	m_vkCmdBuffer.end();

	m_vkQueue.Submit(m_vkCmdBuffer);
	VK_DEVICE.waitIdle();

	m_vkCmdBuffer.Begin(m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX], m_RenderPass);

	//set layout to color attachment for current backbuffer
	if (!m_VKSwapChain.MSAA) {
		m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eColorAttachmentOptimal);
		m_vkCmdBuffer.PushPipelineBarrier();
	}
	
	vk::RenderPassBeginInfo renderPassInfo;
	renderPassInfo.framebuffer = m_VKSwapChain.FrameBuffers[VK_FRAME_INDEX];
	renderPassInfo.renderPass = m_RenderPass;
	glm::vec4 color = glm::vec4(100.0f / 255, 149.0f / 255, 237.0f / 255,1);
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
	renderPassInfo.renderArea = { 0, 0, (uint32_t)m_ScreenSize.x, (uint32_t)m_ScreenSize.y };

	m_vkCmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	//skybox
	m_SkyBox.Render(m_vkCmdBuffer);
	//render here
	vk::DeviceSize offset = 0;
	m_vkCmdBuffer.bindVertexBuffers(0, 1, &m_VertexBuffer.BufferHandle, &offset);
	m_vkCmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipelines[m_CurrentPipeline].GetPipeline());
	m_vkCmdBuffer.setViewport(0, 1, &m_Viewport);
	m_vkCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipelines[m_CurrentPipeline].GetPipelineLayout(), 0, 1, &m_DescriptorSet, 0, nullptr);
	m_vkCmdBuffer.draw(m_MeshVertexCount, 1, 0, 0);
	m_vkCmdBuffer.endRenderPass();

	if (m_MSAA) {
		vk::ImageResolve resolve;
		resolve.dstOffset.x = 0;
		resolve.dstOffset.y = 0;
		resolve.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		resolve.dstSubresource.baseArrayLayer = 0;
		resolve.dstSubresource.layerCount = 1;
		resolve.dstSubresource.mipLevel = 0;
		resolve.extent.width = m_Viewport.width;
		resolve.extent.height = m_Viewport.height;
		resolve.extent.depth = 1.0f;
		resolve.srcOffset.x = 0;
		resolve.srcOffset.y = 0;
		resolve.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		resolve.srcSubresource.baseArrayLayer = 0;
		resolve.srcSubresource.layerCount = 1;
		resolve.srcSubresource.mipLevel = 0;

		m_vkCmdBuffer.resolveImage(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::eColorAttachmentOptimal,
			m_VKSwapChain.ResolveImages[VK_FRAME_INDEX], vk::ImageLayout::ePresentSrcKHR, resolve);

		resolve.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
		resolve.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eDepth;
		m_vkCmdBuffer.resolveImage(m_VKSwapChain.DepthStencilImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilAttachmentOptimal,
			m_VKSwapChain.DepthResolveImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilReadOnlyOptimal, resolve);
	} else {
		m_vkCmdBuffer.ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);
		m_vkCmdBuffer.PushPipelineBarrier();
	}

	m_vkCmdBuffer.end();

	m_vkQueue.Submit(m_vkCmdBuffer, m_ImageAquiredSemaphore, m_RenderCompleteSemaphore, nullptr);
	//raymarch
	m_vkMarchCmdBuffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
	m_vkMarchCmdBuffer.Begin(nullptr, nullptr);

	if (m_VKSwapChain.MSAA) {
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.DepthResolveImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.ResolveImages[VK_FRAME_INDEX], vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral);
	} else {
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.DepthStencilImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::ePresentSrcKHR, vk::ImageLayout::eGeneral);
	}
	m_vkMarchCmdBuffer.PushPipelineBarrier();

	m_Raymarcher.Render(m_vkMarchCmdBuffer, VK_FRAME_INDEX);

	if (m_VKSwapChain.MSAA) {
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.DepthResolveImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.ResolveImages[VK_FRAME_INDEX], vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);
	} else {
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.DepthStencilImages[VK_FRAME_INDEX], vk::ImageLayout::eDepthStencilReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		m_vkMarchCmdBuffer.ImageBarrier(m_VKSwapChain.Images[VK_FRAME_INDEX], vk::ImageLayout::eGeneral, vk::ImageLayout::ePresentSrcKHR);
	}
	m_vkMarchCmdBuffer.PushPipelineBarrier();

	m_vkMarchCmdBuffer.end();
	m_vkQueue.Submit(m_vkMarchCmdBuffer, m_RenderCompleteSemaphore, m_RayMarchComplete, m_Fence[VK_FRAME_INDEX]);
}

void GraphicsEngine::Swap() {

	VK_DEVICE.waitForFences(1, &m_Fence[VK_FRAME_INDEX], true, UINT64_MAX);
	VK_DEVICE.resetFences(1, &m_Fence[VK_FRAME_INDEX]);

	vk::PresentInfoKHR presentInfo = {};
	presentInfo.pImageIndices = &VK_FRAME_INDEX;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_VKSwapChain.SwapChain;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_RayMarchComplete;
	m_vkQueue.presentKHR(presentInfo);

	m_RenderQueues[VK_FRAME_INDEX].Clear();

	VK_FRAME_INDEX = VK_DEVICE.acquireNextImageKHR(m_VKSwapChain.SwapChain, UINT64_MAX, m_ImageAquiredSemaphore, nullptr).value;
}

void GraphicsEngine::NextPipeline() {
	m_CurrentPipeline = (++m_CurrentPipeline) % 3;
}

void GraphicsEngine::PrevPipeline() {
	m_CurrentPipeline = (m_CurrentPipeline + 2) % 3;
}

RenderQueue* GraphicsEngine::GetRenderQueue() {
	return &m_RenderQueues[VK_FRAME_INDEX];
}