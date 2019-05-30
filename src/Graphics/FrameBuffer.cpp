#include "FrameBuffer.h"
#include <Utility/Hash.h>
using namespace smug;

FrameBufferManager::FrameBufferManager() {

}

FrameBufferManager::~FrameBufferManager() {

	for (auto&  rt : m_RenderTargets) {
		vmaDestroyImage(m_DeviceAllocator, rt.second.Handle, rt.second.Memory);
	}
	vmaDestroyAllocator(m_DeviceAllocator);
}

bool IsDepthStencil(VkFormat f) {
	switch (f) {
	case VkFormat::VK_FORMAT_D16_UNORM:
	case VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32:
	case VkFormat::VK_FORMAT_D32_SFLOAT:
	case VkFormat::VK_FORMAT_D16_UNORM_S8_UINT:
	case VkFormat::VK_FORMAT_D24_UNORM_S8_UINT:
	case VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT:
		return true;
		break;
	}
	return false;
}

// Find a memory in `memoryTypeBitsRequirement` that includes all of `requiredProperties`
int32_t findProperties(const VkPhysicalDeviceMemoryProperties* pMemoryProperties, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties) {
	const uint32_t memoryCount = pMemoryProperties->memoryTypeCount;
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex) {
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = memoryTypeBitsRequirement & memoryTypeBits;

		const VkMemoryPropertyFlags properties =
			pMemoryProperties->memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties =
			(properties & requiredProperties) == requiredProperties;

		if (isRequiredMemoryType && hasRequiredProperties)
			return static_cast<int32_t>(memoryIndex);
	}

	// failed to find memory type
	return -1;
}

void FrameBufferManager::Init(const VkDevice& device, const VkPhysicalDevice& gpu, const glm::vec2& size, const eastl::vector<VkFormat>& formats, const eastl::vector<VkImageUsageFlags>& usages) {
	m_Device = device;
	m_FrameBufferSize = size;
	m_FormatCount = (uint32_t)formats.size();
	m_Images.resize(m_FormatCount);
	//init image and views
	for (uint32_t f = 0; f < m_FormatCount; ++f) {
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pNext = nullptr;
		imageInfo.arrayLayers = 1;
		imageInfo.extent = { (uint32_t)size.x, (uint32_t)size.y, 1 };
		imageInfo.format = formats[f];
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.mipLevels = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; //dont support msaa for now
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = usages[f];
		vkCreateImage(device, &imageInfo, nullptr, &m_Images[f]);

		m_CurrentLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED);
	}

	//init memory
	uint64_t memSize = 0;
	uint32_t memBits = 0;
	uint32_t imageCount = (uint32_t)m_Images.size();
	for (uint32_t i = 0; i < imageCount; ++i) {
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device, m_Images[i], &memReq);
		memBits |= memReq.memoryTypeBits;
		memSize = (memSize + (memReq.alignment - 1)) & ~(memReq.alignment - 1); //handle alignment spill
		memSize += memReq.size;
	}
	VkMemoryPropertyFlagBits deviceMemFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(gpu, &memProps);

	int32_t memIndex = findProperties(&memProps, memBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.allocationSize = memSize;
	allocInfo.memoryTypeIndex = memIndex;
	vkAllocateMemory(device, &allocInfo, nullptr, &m_Memory);
	m_MemorySize = memSize;

	uint32_t image = 0;
	uint64_t memOffset = 0;
	m_ImageViews.resize(m_FormatCount);
	for (uint32_t f = 0; f < m_FormatCount; ++f) {
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(device, m_Images[image], &memReq);
		memOffset = (memOffset + (memReq.alignment - 1)) & ~(memReq.alignment - 1); //handle alignment
		vkBindImageMemory(device, m_Images[image], m_Memory, memOffset);
		memOffset += memReq.size;

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.components = { VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G,VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B, VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A };
		imageViewInfo.format = formats[f];
		imageViewInfo.image = m_Images[image];
		imageViewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.subresourceRange.aspectMask = IsDepthStencil(formats[f]) ? VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT : VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT; //for now assume that the stencil is not gonna be read in a later pass
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;

		vkCreateImageView(device, &imageViewInfo, nullptr, &m_ImageViews[f]);
		image++;
	}

	//init renderpass
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = (uint32_t)formats.size();
	renderPassInfo.dependencyCount = 0; //future optimization, get dependency off another framebuffer
	eastl::vector<VkAttachmentDescription> attachments;
	for (uint32_t f = 0; f < m_FormatCount; ++f) {
		VkAttachmentDescription attachmentDesc = {};
		attachmentDesc.format = formats[f];
		VkImageLayout layout = IsDepthStencil(formats[f]) ? VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachmentDesc.finalLayout = layout;
		attachmentDesc.initialLayout = layout;
		attachmentDesc.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDesc.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDesc.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDesc.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDesc.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT; //dont support msaa for now
		attachments.push_back(attachmentDesc);
	}
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	//init subpass
	VkSubpassDescription subPassDesc = {};
	eastl::vector<VkAttachmentReference> colorAttachments;
	eastl::vector<VkAttachmentReference> depthAttachments;
	for (uint32_t i = 0; i < attachments.size(); ++i) {
		if (attachments[i].initialLayout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
			colorAttachments.push_back({ i, VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
		} else {
			depthAttachments.push_back({ i, VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL });
		}
	}
	assert(depthAttachments.size() <= 1);
	subPassDesc.colorAttachmentCount = (uint32_t)colorAttachments.size();
	subPassDesc.pColorAttachments = colorAttachments.data();
	subPassDesc.pDepthStencilAttachment = depthAttachments.data();
	subPassDesc.inputAttachmentCount = 0;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.pPreserveAttachments = nullptr;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	subPassDesc.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;

	vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass);

	//init framebuffer
	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.pNext = nullptr;
	framebufferInfo.attachmentCount = (uint32_t)formats.size();
	framebufferInfo.height = (uint32_t)size.y;
	framebufferInfo.width = (uint32_t)size.x;
	framebufferInfo.layers = 1;
	framebufferInfo.renderPass = m_RenderPass;
	framebufferInfo.pAttachments = m_ImageViews.data();

	vkCreateFramebuffer(device, &framebufferInfo, nullptr, &m_FrameBuffer);

	//Init descriptors
	for (uint32_t i = 0; i < m_FormatCount; ++i) {
		VkDescriptorImageInfo imageDescInfo = {};
		if (usages[i] & VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT) {
			imageDescInfo.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageDescInfo.imageView = m_ImageViews[i];

			VkSamplerCreateInfo samplerInfo = {};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.pNext = nullptr;
			samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = false;
			samplerInfo.magFilter = VkFilter::VK_FILTER_NEAREST;
			samplerInfo.minFilter = VkFilter::VK_FILTER_NEAREST;
			samplerInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
			if (IsDepthStencil(formats[i])) {
				//used for shadow maps
				samplerInfo.compareEnable = true;
				samplerInfo.compareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
				samplerInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
				samplerInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
				samplerInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			} else {
				samplerInfo.compareEnable = false;
			}
			vkCreateSampler(device, &samplerInfo, nullptr, &imageDescInfo.sampler);
		}
		m_Descriptors.push_back(imageDescInfo);
	}

	m_Formats.insert(m_Formats.end(), formats.begin(), formats.end());


	VmaAllocatorCreateInfo allocCreateInfo = {};
	allocCreateInfo.device = m_Device;
	allocCreateInfo.pAllocationCallbacks = nullptr;
	allocCreateInfo.pDeviceMemoryCallbacks = nullptr;
	allocCreateInfo.pHeapSizeLimit = nullptr;
	allocCreateInfo.physicalDevice = gpu;
	allocCreateInfo.pRecordSettings = nullptr;
	allocCreateInfo.preferredLargeHeapBlockSize = 0;
	VmaVulkanFunctions vkFunks;
	vkFunks.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
	vkFunks.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
	vkFunks.vkAllocateMemory = vkAllocateMemory;
	vkFunks.vkFreeMemory = vkFreeMemory;
	vkFunks.vkMapMemory = vkMapMemory;
	vkFunks.vkUnmapMemory = vkUnmapMemory;
	vkFunks.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
	vkFunks.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
	vkFunks.vkBindBufferMemory = vkBindBufferMemory;
	vkFunks.vkBindImageMemory = vkBindImageMemory;
	vkFunks.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
	vkFunks.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
	vkFunks.vkCreateBuffer = vkCreateBuffer;
	vkFunks.vkDestroyBuffer = vkDestroyBuffer;
	vkFunks.vkCreateImage = vkCreateImage;
	vkFunks.vkDestroyImage = vkDestroyImage;
	vkFunks.vkCmdCopyBuffer = vkCmdCopyBuffer;
	vkFunks.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR;
	vkFunks.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR;
	allocCreateInfo.pVulkanFunctions = &vkFunks;
	vmaCreateAllocator(&allocCreateInfo,&m_DeviceAllocator);

}

void FrameBufferManager::Resize(const glm::vec2& size) {
	//TODO
}

///Push barrier needs to be called for this to make an effect
void FrameBufferManager::ChangeLayout(CommandBuffer& cmdBuffer, const eastl::vector<VkImageLayout>& newLayouts) {

	uint32_t imageCount = (uint32_t)m_Formats.size();
	assert(imageCount == newLayouts.size());

	for (uint32_t i = 0; i < imageCount; ++i) {
		cmdBuffer.ImageBarrier(m_Images[i], m_CurrentLayouts[i], newLayouts[i]);
		m_CurrentLayouts[i] = newLayouts[i];
	}
}

void FrameBufferManager::ChangeLayout(CommandBuffer& cmdBuffer, uint32_t renderTarget, VkImageLayout newLayout) {
	RenderTarget& rt = m_RenderTargets[renderTarget];
	if (rt.Layout == newLayout) {
		return;
	}
	cmdBuffer.ImageBarrier(rt.Handle, rt.Layout, newLayout);
	rt.Layout = newLayout;
}

void FrameBufferManager::SetLayouts(const eastl::vector<VkImageLayout>& newLayouts) {
	uint32_t size = (uint32_t)newLayouts.size();
	for (uint32_t i = 0; i < size; ++i) {
		m_CurrentLayouts[i] = newLayouts[i];
	}
}

void FrameBufferManager::AllocRenderTarget(uint32_t name, uint32_t width, uint32_t height, uint32_t depth, VkFormat format) {
	RenderTarget rt;
	rt.Name = name;
	rt.Format = format;
	rt.Width = width;
	rt.Height = height;
	rt.Depth = depth;
	rt.Layout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.extent = { width, height, depth };
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.imageType = VkImageType::VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.mipLevels = 1;
	imageInfo.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.tiling = VkImageTiling::VK_IMAGE_TILING_OPTIMAL;
	if (IsDepthStencil(format)) {
		imageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
	} else {
		imageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_STORAGE_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageInfo.usage |= VkImageUsageFlagBits::VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	vkCreateImage(m_Device, &imageInfo, nullptr, &rt.Handle);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	allocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	VmaAllocationInfo info;
	if (vmaAllocateMemoryForImage(m_DeviceAllocator, rt.Handle, &allocInfo, &rt.Memory, &info)) {
		printf("Error! Could not allocate memory for render target %u\n", name);
		return;
	}
	vmaBindImageMemory(m_DeviceAllocator, rt.Memory, rt.Handle);

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.components = { VkComponentSwizzle::VK_COMPONENT_SWIZZLE_R, VkComponentSwizzle::VK_COMPONENT_SWIZZLE_G, VkComponentSwizzle::VK_COMPONENT_SWIZZLE_B, VkComponentSwizzle::VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = format;
	viewInfo.image = rt.Handle;
	if (IsDepthStencil(format)) {
		viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
	} else {
		viewInfo.subresourceRange.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	}
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D;

	vkCreateImageView(m_Device, &viewInfo, nullptr, &rt.View);

	m_RenderTargets[name] = rt;
}

VkRenderPass FrameBufferManager::CreateRenderPass(uint32_t name, const eastl::vector<uint32_t>& renderTargets) {
	
	eastl::vector<VkAttachmentDescription> attachments;
	uint32_t depthStencil = UINT_MAX;
	eastl::vector<VkSubpassDescription> subPassesDescs;
	for (uint32_t i = 0; i < renderTargets.size(); ++i) {
		VkImageLayout layout = IsDepthStencil(m_RenderTargets[renderTargets[i]].Format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentDescription a = {};
		a.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED; //we handle transitions ourselves with barriers
		a.finalLayout = layout;
		a.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		a.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		a.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		a.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		a.samples = VK_SAMPLE_COUNT_1_BIT;
		a.format = m_RenderTargets[renderTargets[i]].Format;
		if (IsDepthStencil(a.format))
			depthStencil = i;

		attachments.push_back(a);
	}
	VkAttachmentReference depthStencilRef;
	eastl::vector<VkAttachmentReference> attachmentRefs;
	for (uint32_t i = 0; i < renderTargets.size(); ++i) {
		VkImageLayout layout = IsDepthStencil(m_RenderTargets[renderTargets[i]].Format) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkAttachmentReference ref = {};
		ref.attachment = i;
		ref.layout = layout;
		if (i == depthStencil) {
			depthStencilRef = ref;
		} else {
			attachmentRefs.push_back(ref);
		}
	}
	VkSubpassDescription spDesc = {};
	spDesc.inputAttachmentCount = 0;
	spDesc.pInputAttachments = nullptr;
	spDesc.preserveAttachmentCount = 0;
	spDesc.pPreserveAttachments = nullptr;
	spDesc.pResolveAttachments = nullptr;
	spDesc.pColorAttachments = attachmentRefs.data();
	spDesc.colorAttachmentCount = attachmentRefs.size();
	spDesc.pDepthStencilAttachment = &depthStencilRef;

	subPassesDescs.push_back(spDesc);

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.pDependencies = nullptr;
	renderPassInfo.pSubpasses = subPassesDescs.data();
	renderPassInfo.subpassCount = (uint32_t)subPassesDescs.size();

	vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPasses[name]);

	uint32_t w = 0, h = 0, d = 0;
	eastl::vector<VkImageView> fbViews;
	for (auto& rt : renderTargets) {
		fbViews.push_back(m_RenderTargets[rt].View);
		w = glm::max(w, m_RenderTargets[rt].Width);
		h = glm::max(h, m_RenderTargets[rt].Height);
		d = glm::max(d, m_RenderTargets[rt].Depth);
	}

	VkFramebufferCreateInfo fbInfo = {};
	fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbInfo.pNext = nullptr;
	fbInfo.attachmentCount = (uint32_t)fbViews.size();
	fbInfo.pAttachments = fbViews.data();
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = d;
	fbInfo.renderPass = m_RenderPasses[name];

	vkCreateFramebuffer(m_Device, &fbInfo, nullptr, &m_FrameBuffers[name]);

	return m_RenderPasses[name];
}