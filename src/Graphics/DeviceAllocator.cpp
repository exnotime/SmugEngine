#include "DeviceAllocator.h"
using namespace smug;

DeviceAllocator::DeviceAllocator() {

}
DeviceAllocator::~DeviceAllocator() {
	vmaDestroyAllocator(m_Allocator);
}
void DeviceAllocator::Init(vk::Device& device, vk::PhysicalDevice& physicalDevice) {
	VmaAllocatorCreateInfo allocatorCrateInfo = {};
	allocatorCrateInfo.device = device;
	allocatorCrateInfo.physicalDevice = physicalDevice;
	vmaCreateAllocator(&allocatorCrateInfo, &m_Allocator);

	m_Device = &device;
}
void DeviceAllocator::AllocateImage(VkImageCreateInfo* createInfo, VkImage* image, uint64_t size, void* data) {
	VkImageLayout finalLayout = createInfo->initialLayout;
	createInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaMemoryRequirements memReqs = {};
	memReqs.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkResult vkres = vmaCreateImage(m_Allocator, createInfo, &memReqs, image, nullptr, nullptr);
	if (vkres != VK_SUCCESS) {
		printf("Error allocating memory for image\n");
		return;
	}
	if (data) {
		//create intermediate texture to copy from
		memReqs.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		VkMappedMemoryRange memMap;
		VkBuffer intImage;
		VkMemoryRequirements vkMemReqs;
		vkGetImageMemoryRequirements(*m_Device, *image, &vkMemReqs);

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.size = vkMemReqs.size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult vkres = vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &memReqs, &intImage, &memMap, nullptr);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate image\n");
			return;
		}
		void* ptr;
		vmaMapMemory(m_Allocator, &memMap, &ptr);
		memcpy(ptr, data, size);
		vkFlushMappedMemoryRanges(*m_Device, 1, &memMap);
		vmaUnmapMemory(m_Allocator, &memMap);

		ImageTransfer transfer;
		uint64_t bufferOffset = 0;
		uint32_t w, h;
		for (uint32_t l = 0; l < createInfo->arrayLayers; ++l) {
			w = createInfo->extent.width;
			h = createInfo->extent.height;
			for (uint32_t mip = 0; mip < createInfo->mipLevels; mip++) {
				VkBufferImageCopy imageCopy = {};
				imageCopy.imageExtent = {w,h,1};
				imageCopy.bufferOffset = bufferOffset;
				imageCopy.bufferRowLength = 0; //TODO: compressed formats have different
				imageCopy.bufferImageHeight = 0;
				imageCopy.imageOffset = { 0,0,0 };
				imageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageCopy.imageSubresource.baseArrayLayer = l;
				imageCopy.imageSubresource.mipLevel = mip;
				imageCopy.imageSubresource.layerCount = 1;
				bufferOffset += w * h * 4; //temp
				w >>= 1;
				h >>= 1;
				transfer.copies.push_back(imageCopy);
			}
		}

		transfer.src = intImage;
		transfer.dst = *image;
		transfer.finalLayout = finalLayout;
		m_ImageCopies.push_back(transfer);
	}
}

VkBufferHandle DeviceAllocator::AllocateBuffer(const VkBufferUsageFlags usage, uint64_t size, void * data) {
	VkBufferHandle ret;

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaMemoryRequirements memReqs = {};
	memReqs.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkResult vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &ret.buffer, &ret.memory, nullptr);
	if (vkres != VK_SUCCESS) {
		printf("Error allocating memory for buffer\n");
		return ret;
	}
	if (data) {
		BufferTransfer transfer;
		memReqs.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMappedMemoryRange memoryRange;
		vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &transfer.src, &memoryRange, nullptr);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate buffer\n");
			return ret;
		}

		void* ptr;
		vmaMapMemory(m_Allocator, &memoryRange, &ptr);
		memcpy(ptr, data, size);
		vkFlushMappedMemoryRanges(*m_Device, 1, &memoryRange);
		vmaUnmapMemory(m_Allocator, &memoryRange);

		transfer.dst = ret.buffer;
		transfer.copy.dstOffset = 0;
		transfer.copy.srcOffset = 0;
		transfer.copy.size = size;
		m_BufferCopies.push_back(transfer);
	}
	return ret;
}

void DeviceAllocator::UpdateBuffer(VkBufferHandle& handle, uint64_t size, void* data) {
	if (data) {
		BufferTransfer transfer;
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.size = size;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaMemoryRequirements memReqs = {};

		memReqs.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		VkMappedMemoryRange memoryRange;
		VkResult vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &transfer.src, &memoryRange, nullptr);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate buffer\n");
			return;
		}

		void* ptr;
		vmaMapMemory(m_Allocator, &memoryRange, &ptr);
		memcpy(ptr, data, size);
		vkFlushMappedMemoryRanges(*m_Device, 1, &memoryRange);
		vmaUnmapMemory(m_Allocator, &memoryRange);

		transfer.dst = handle.buffer;
		transfer.copy.dstOffset = 0;
		transfer.copy.srcOffset = 0;
		transfer.copy.size = size;
		m_BufferCopies.push_back(transfer);
	}
}

void DeviceAllocator::DeAllocateBuffer(VkBufferHandle& handle) {
	vmaDestroyBuffer(m_Allocator, handle.buffer);
}

void DeviceAllocator::DeAllocateImage(VkImage& image) {
	vmaDestroyImage(m_Allocator, image);
}

void DeviceAllocator::ScheduleTransfers(CommandBuffer* cmdBuffer) {
	uint32_t imageCount = (uint32_t)m_ImageCopies.size();
	if (imageCount > 0) {
		for (uint32_t i = 0; i < imageCount; ++i) {
			cmdBuffer->ImageBarrier(m_ImageCopies[i].dst, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		}
		cmdBuffer->PushPipelineBarrier();
		for (uint32_t i = 0; i < imageCount; ++i) {
			cmdBuffer->copyBufferToImage(m_ImageCopies[i].src, m_ImageCopies[i].dst, vk::ImageLayout::eTransferDstOptimal, m_ImageCopies[i].copies);
			cmdBuffer->ImageBarrier(m_ImageCopies[i].dst, vk::ImageLayout::eTransferDstOptimal, (vk::ImageLayout)m_ImageCopies[i].finalLayout);
		}
		cmdBuffer->PushPipelineBarrier();
		//m_ImageCopies.clear();
	}

	uint32_t bufferCopies = (uint32_t)m_BufferCopies.size();
	if (bufferCopies > 0) {
		for (uint32_t i = 0; i < bufferCopies; ++i) {
			std::array<vk::BufferCopy, 1> copy = { m_BufferCopies[i].copy };
			cmdBuffer->copyBuffer(m_BufferCopies[i].src, m_BufferCopies[i].dst, copy);
		}
		//m_BufferCopies.clear();
	}
	cmdBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, nullptr);
}

void DeviceAllocator::Clear() {
	for (auto& buffer : m_BufferCopies) {
		vmaDestroyBuffer(m_Allocator, buffer.src);
	}
	for (auto& buffer : m_ImageCopies) {
		vmaDestroyBuffer(m_Allocator, buffer.src);
	}
	m_BufferCopies.clear();
	m_ImageCopies.clear();
}

void DeviceAllocator::PrintStats() {

}