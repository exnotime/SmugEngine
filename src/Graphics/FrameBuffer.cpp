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

bool IsDepthStencil(vk::Format f) {
	switch (f) {
	case vk::Format::eD16Unorm:
	case vk::Format::eX8D24UnormPack32:
	case vk::Format::eD32Sfloat:
	case vk::Format::eD16UnormS8Uint:
	case vk::Format::eD24UnormS8Uint:
	case vk::Format::eD32SfloatS8Uint:
		return true;
		break;
	}
	return false;
}

void FrameBufferManager::Init(const vk::Device& device, const vk::PhysicalDevice& gpu, const glm::vec2& size, const std::vector<vk::Format>& formats, const std::vector<vk::ImageUsageFlags>& usages, uint32_t bufferCount) {
	m_Device = device;
	m_FrameBufferSize = size;
	m_FormatCount = (uint32_t)formats.size();
	//init image and views
	for (uint32_t i = 0; i < bufferCount; i++) {
		for (uint32_t f = 0; f < m_FormatCount; ++f) {
			vk::ImageCreateInfo imageInfo;
			imageInfo.arrayLayers = 1;
			imageInfo.extent = { (uint32_t)size.x, (uint32_t)size.y, 1 };
			imageInfo.format = formats[f];
			imageInfo.imageType = vk::ImageType::e2D;
			imageInfo.initialLayout = vk::ImageLayout::eUndefined;
			imageInfo.mipLevels = 1;
			imageInfo.samples = vk::SampleCountFlagBits::e1; //dont support msaa for now
			imageInfo.sharingMode = vk::SharingMode::eExclusive;
			imageInfo.tiling = vk::ImageTiling::eOptimal;
			imageInfo.usage = usages[f];
			m_Images.push_back(device.createImage(imageInfo));
			m_CurrentLayouts.push_back(vk::ImageLayout::eUndefined);
		}
	}

	//init memory
	uint64_t memSize = 0;
	uint32_t memBits = 0;
	uint32_t imageCount = (uint32_t)m_Images.size();
	for (uint32_t i = 0; i < imageCount; ++i) {
		auto memReq = device.getImageMemoryRequirements(m_Images[i]);
		memBits |= memReq.memoryTypeBits;
		memSize = (memSize + (memReq.alignment - 1)) & ~(memReq.alignment - 1); //handle alignment spill
		memSize += memReq.size;
	}
	vk::MemoryPropertyFlagBits deviceMemFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;
	auto& memProps = gpu.getMemoryProperties();
	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
		if ((memProps.memoryTypes[i].propertyFlags & deviceMemFlags) == deviceMemFlags && memBits & (1 << i)) {
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = memSize;
			allocInfo.memoryTypeIndex = i;
			m_Memory = device.allocateMemory(allocInfo);
			m_MemorySize = memSize;
			break;
		}
	}

	uint32_t image = 0;
	uint64_t memOffset = 0;
	for (uint32_t i = 0; i < bufferCount; i++) {
		for (uint32_t f = 0; f < m_FormatCount; ++f) {
			auto memReq = device.getImageMemoryRequirements(m_Images[image]);
			memOffset = (memOffset + (memReq.alignment - 1)) & ~(memReq.alignment - 1); //handle alignment
			device.bindImageMemory(m_Images[image], m_Memory, memOffset);
			memOffset += memReq.size;

			vk::ImageViewCreateInfo imageViewInfo;
			imageViewInfo.components = { vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
			imageViewInfo.format = formats[f];
			imageViewInfo.image = m_Images[image];
			imageViewInfo.viewType = vk::ImageViewType::e2D;
			imageViewInfo.subresourceRange.aspectMask = IsDepthStencil(formats[f]) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor; //for now assume that the stencil is not gonna be read in a later pass
			imageViewInfo.subresourceRange.baseArrayLayer = 0;
			imageViewInfo.subresourceRange.baseMipLevel = 0;
			imageViewInfo.subresourceRange.layerCount = 1;
			imageViewInfo.subresourceRange.levelCount = 1;

			m_ImageViews.push_back(device.createImageView(imageViewInfo));
			image++;
		}
	}

	//init renderpass
	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = (uint32_t)formats.size();
	renderPassInfo.dependencyCount = 0; //future optimization, get dependency off another framebuffer
	std::vector<vk::AttachmentDescription> attachments;
	for (uint32_t f = 0; f < m_FormatCount; ++f) {
		vk::AttachmentDescription attachmentDesc;
		attachmentDesc.format = formats[f];
		vk::ImageLayout layout = IsDepthStencil(formats[f]) ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;
		attachmentDesc.finalLayout = layout;
		attachmentDesc.initialLayout = layout;
		attachmentDesc.loadOp = vk::AttachmentLoadOp::eClear;
		attachmentDesc.storeOp = vk::AttachmentStoreOp::eStore;
		attachmentDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachmentDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachmentDesc.samples = vk::SampleCountFlagBits::e1; //dont support msaa for now
		attachments.push_back(attachmentDesc);
	}
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.pAttachments = attachments.data();
	//init subpass
	vk::SubpassDescription subPassDesc;
	std::vector<vk::AttachmentReference> colorAttachments;
	std::vector<vk::AttachmentReference> depthAttachments;
	for (uint32_t i = 0; i < attachments.size(); ++i) {
		if (attachments[i].initialLayout == vk::ImageLayout::eColorAttachmentOptimal) {
			colorAttachments.push_back({ i, vk::ImageLayout::eColorAttachmentOptimal });
		} else {
			depthAttachments.push_back({ i, vk::ImageLayout::eDepthStencilAttachmentOptimal });
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
	subPassDesc.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subPassDesc;

	m_RenderPass = device.createRenderPass(renderPassInfo);

	//init framebuffer
	vk::FramebufferCreateInfo framebufferInfo;
	framebufferInfo.attachmentCount = (uint32_t)formats.size();
	framebufferInfo.height = (uint32_t)size.y;
	framebufferInfo.width = (uint32_t)size.x;
	framebufferInfo.layers = 1;
	framebufferInfo.renderPass = m_RenderPass;
	for (uint32_t i = 0; i < bufferCount; i++) {
		framebufferInfo.pAttachments = &m_ImageViews[i * framebufferInfo.attachmentCount];
		m_FrameBuffers[i] = device.createFramebuffer(framebufferInfo);
	}

	//Init descriptors
	for (uint32_t i = 0; i < m_FormatCount; ++i) {
		vk::DescriptorImageInfo imageDescInfo = {};
		if (usages[i] & vk::ImageUsageFlagBits::eSampled) {
			imageDescInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			imageDescInfo.imageView = m_ImageViews[i];

			vk::SamplerCreateInfo samplerInfo = {};
			samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
			samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
			samplerInfo.anisotropyEnable = false;
			samplerInfo.magFilter = vk::Filter::eNearest;
			samplerInfo.minFilter = vk::Filter::eNearest;
			samplerInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
			if (IsDepthStencil(formats[i])) {
				//used for shadow maps
				samplerInfo.compareEnable = true;
				samplerInfo.compareOp = vk::CompareOp::eLessOrEqual;
				samplerInfo.magFilter = vk::Filter::eLinear;
				samplerInfo.minFilter = vk::Filter::eLinear;
				samplerInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
				samplerInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
				samplerInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
			} else {
				samplerInfo.compareEnable = false;
			}
			imageDescInfo.sampler = device.createSampler(samplerInfo);
		}
		m_Descriptors.push_back(imageDescInfo);
	}

	m_Formats.insert(m_Formats.end(), formats.begin(), formats.end());

	m_BufferCount = bufferCount;


	VmaAllocatorCreateInfo allocCreateInfo;
	allocCreateInfo.device = m_Device;
	allocCreateInfo.pAllocationCallbacks = nullptr;
	allocCreateInfo.pDeviceMemoryCallbacks = nullptr;
	allocCreateInfo.pHeapSizeLimit = nullptr;
	allocCreateInfo.physicalDevice = gpu;
	allocCreateInfo.pRecordSettings = nullptr;
	allocCreateInfo.preferredLargeHeapBlockSize = 0;
	allocCreateInfo.pVulkanFunctions = nullptr;
	vmaCreateAllocator(&allocCreateInfo,&m_DeviceAllocator);

}

void FrameBufferManager::Resize(const glm::vec2& size) {
	//TODO
}

///Push barrier needs to be called for this to make an effect
void FrameBufferManager::ChangeLayout(CommandBuffer& cmdBuffer, const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex) {

	uint32_t imageCount = (uint32_t)m_Formats.size();
	assert(imageCount == newLayouts.size());
	assert(frameIndex < m_BufferCount);

	for (uint32_t i = 0; i < imageCount; ++i) {
		cmdBuffer.ImageBarrier(m_Images[frameIndex * imageCount + i], m_CurrentLayouts[frameIndex * imageCount + i], newLayouts[i]);
		m_CurrentLayouts[frameIndex * imageCount + i] = newLayouts[i];
	}
}

void FrameBufferManager::SetLayouts(const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex) {
	uint32_t size = (uint32_t)newLayouts.size();
	uint32_t begin = m_FormatCount * frameIndex;
	for (uint32_t i = begin; i < size; ++i) {
		m_CurrentLayouts[begin + i] = newLayouts[i];
	}
}

void FrameBufferManager::AllocRenderTarget(uint32_t name, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageLayout initialLayout) {
	RenderTarget rt;
	rt.Name = name;
	rt.Format = format;
	rt.Width = width;
	rt.Height = height;
	rt.Depth = depth;
	rt.Layout = initialLayout;

	vk::ImageCreateInfo imageInfo = {};
	imageInfo.extent = { width, height, depth };
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.imageType = vk::ImageType::e2D;
	imageInfo.initialLayout = vk::ImageLayout::eUndefined;
	imageInfo.mipLevels = 1;
	imageInfo.samples = vk::SampleCountFlagBits::e1;
	imageInfo.sharingMode = vk::SharingMode::eExclusive;
	imageInfo.tiling = vk::ImageTiling::eOptimal;
	if (IsDepthStencil(format)) {
		imageInfo.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment;
		imageInfo.usage |= vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
	} else {
		imageInfo.usage = vk::ImageUsageFlagBits::eColorAttachment;
		imageInfo.usage |= vk::ImageUsageFlagBits::eInputAttachment;
		imageInfo.usage |= vk::ImageUsageFlagBits::eSampled;
		imageInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
		imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
	}
	rt.Handle = m_Device.createImage(imageInfo);

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	allocInfo.usage = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
	VmaAllocationInfo info;
	if (vmaAllocateMemoryForImage(m_DeviceAllocator, rt.Handle, &allocInfo, &rt.Memory, &info)) {
		printf("Error! Could not allocate memory for render target %u\n", name);
		return;
	}
	vmaBindImageMemory(m_DeviceAllocator, rt.Memory, rt.Handle);

	vk::ImageViewCreateInfo viewInfo;
	viewInfo.components = { vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA };
	viewInfo.format = format;
	viewInfo.image = rt.Handle;
	if (IsDepthStencil(format)) {
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
	} else {
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.viewType = vk::ImageViewType::e2D;

	rt.View = m_Device.createImageView(viewInfo);

	m_RenderTargets[name] = rt;
}

vk::RenderPass FrameBufferManager::CreateRenderPass(uint32_t name, std::vector<SubPass> subPasses) {
	//create hash and see if we already have a renderpass that matches
	std::vector<uint32_t> subpassHashes;
	uint32_t rpHash;
	MurmurHash3_x86_32(subpassHashes.data(), sizeof(uint32_t) * subpassHashes.size(),0xBEEFC0DE, (void*)&rpHash);

	std::vector<vk::AttachmentDescription> attachments;
	std::unordered_map<uint32_t, uint32_t> nameToIndex;
	std::vector<std::vector<vk::AttachmentReference>> attachmentRefs;
	attachmentRefs.resize(subPasses.size());
	std::vector<vk::SubpassDescription> subPassesDescs;
	uint32_t subPassIndex = 0;
	for (auto& sp : subPasses) {
		for (auto& rt : sp.RenderTargets) {
			if (nameToIndex.find(rt) != nameToIndex.end()) {
				vk::AttachmentDescription a;
				a.initialLayout = vk::ImageLayout::eUndefined; //we handle transitions ourselves with barriers
				a.finalLayout = vk::ImageLayout::eUndefined;
				a.loadOp = vk::AttachmentLoadOp::eLoad;
				a.storeOp = vk::AttachmentStoreOp::eStore;
				a.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
				a.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
				a.format = m_RenderTargets[rt].Format;
				nameToIndex[rt] = (uint32_t)attachments.size();
				attachments.push_back(a);
			}
		}
		if (sp.DepthStencilAttachment != UINT_MAX) {
			vk::AttachmentDescription a;
			a.initialLayout = vk::ImageLayout::eUndefined; //we handle transitions ourselves with barriers
			a.finalLayout = vk::ImageLayout::eUndefined;
			a.loadOp = vk::AttachmentLoadOp::eLoad;
			a.storeOp = vk::AttachmentStoreOp::eStore;
			a.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
			a.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
			a.format = m_RenderTargets[sp.DepthStencilAttachment].Format;
			nameToIndex[sp.DepthStencilAttachment] = (uint32_t)attachments.size();
			attachments.push_back(a);
		}
	}
	for (auto& sp : subPasses) {
		vk::SubpassDescription spDesc;
		spDesc.inputAttachmentCount = 0;
		spDesc.pInputAttachments = nullptr;
		spDesc.preserveAttachmentCount = 0;
		spDesc.pPreserveAttachments = nullptr;
		spDesc.pResolveAttachments = nullptr;
		for (uint32_t i = 0; i < sp.RenderTargets.size(); ++i) {
			vk::AttachmentReference ref;
			ref.attachment = nameToIndex[sp.RenderTargets[i]];
			ref.layout = vk::ImageLayout::eUndefined;
			attachmentRefs[subPassIndex].push_back(ref);
		}
		spDesc.colorAttachmentCount = (uint32_t)sp.RenderTargets.size();
		spDesc.pDepthStencilAttachment = attachmentRefs[subPassIndex].data();
		if (sp.DepthStencilAttachment != UINT_MAX) {
			vk::AttachmentReference ref;
			ref.attachment = nameToIndex[sp.RenderTargets[sp.DepthStencilAttachment]];
			ref.layout = vk::ImageLayout::eUndefined;
			attachmentRefs[subPassIndex].push_back(ref);
			spDesc.pDepthStencilAttachment = &attachmentRefs[subPassIndex].back();
		}
		subPassesDescs.push_back(spDesc);
	}

	vk::RenderPassCreateInfo renderPassInfo;
	renderPassInfo.attachmentCount = (uint32_t)attachments.size();
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.pDependencies = nullptr;
	renderPassInfo.pSubpasses = subPassesDescs.data();
	renderPassInfo.subpassCount = (uint32_t)subPassesDescs.size();

	m_RenderPasses[name] = m_Device.createRenderPass(renderPassInfo);

	uint32_t w = 0, h = 0, d = 0;
	std::vector<vk::ImageView> fbViews;
	for (auto& sp : subPasses) {
		for (auto& rt : sp.RenderTargets) {
			fbViews.push_back(m_RenderTargets[rt].View);
			w = glm::max(w, m_RenderTargets[rt].Width);
			h = glm::max(h, m_RenderTargets[rt].Height);
			d = glm::max(d, m_RenderTargets[rt].Depth);
		}
		if (sp.DepthStencilAttachment != UINT_MAX) {
			fbViews.push_back(m_RenderTargets[sp.DepthStencilAttachment].View);
		}

	}

	vk::FramebufferCreateInfo fbInfo;
	fbInfo.attachmentCount = (uint32_t)fbViews.size();
	fbInfo.pAttachments = fbViews.data();
	fbInfo.width = w;
	fbInfo.height = h;
	fbInfo.layers = d;
	fbInfo.renderPass = m_RenderPasses[name];
	m_FrameBuffersT[name] = m_Device.createFramebuffer(fbInfo);

	return m_RenderPasses[name];
}