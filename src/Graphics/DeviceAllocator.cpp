#include "DeviceAllocator.h"

using namespace smug;

DeviceAllocator::DeviceAllocator() {

}
DeviceAllocator::~DeviceAllocator() {
	vmaDestroyAllocator(m_Allocator);
}

void DeviceAllocator::Init(VkDevice& device, VkPhysicalDevice& physicalDevice) {
	VmaAllocatorCreateInfo allocatorCrateInfo = {};
	allocatorCrateInfo.device = device;
	allocatorCrateInfo.physicalDevice = physicalDevice;
	//VmaRecordSettings recordSettings;
	//recordSettings.pFilePath = "./vmadata.csv";
	//allocatorCrateInfo.pRecordSettings = &recordSettings;
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
	allocatorCrateInfo.pVulkanFunctions = &vkFunks;
	vmaCreateAllocator(&allocatorCrateInfo, &m_Allocator);

	m_Device = &device;
}
VkImageHandle DeviceAllocator::AllocateImage(VkImageCreateInfo* createInfo, uint64_t size, void* data) {
	VkImageLayout finalLayout = createInfo->initialLayout;
	createInfo->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImageHandle handle;
	VkResult vkres = vmaCreateImage(m_Allocator, createInfo, &allocInfo, &handle.image, &handle.memory, nullptr);
	if (vkres != VK_SUCCESS) {
		printf("Error allocating memory for image\n");
		return handle;
	}
	m_ImageCounter++;
	if (data) {
		//create intermediate texture to copy from
		VmaAllocationInfo info;
		VmaAllocation memMap;
		VkBuffer intImage;
		VkMemoryRequirements vkMemReqs;
		vkGetImageMemoryRequirements(*m_Device, handle.image, &vkMemReqs);

		VkBufferCreateInfo bufferCreateInfo = {};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.pNext = nullptr;
		bufferCreateInfo.size = vkMemReqs.size;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VkResult vkres = vmaCreateBuffer(m_Allocator, &bufferCreateInfo, &allocInfo, &intImage, &memMap, &info);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate image\n");
			return handle;
		}
		m_BufferCounter++;

		void* ptr = nullptr;
		vmaMapMemory(m_Allocator, memMap, &ptr);
		memcpy(ptr, data, size);
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = nullptr;
		range.memory = info.deviceMemory;
		range.offset = info.offset;
		range.size = info.size;
		vkFlushMappedMemoryRanges(*m_Device, 1, &range);
		vmaUnmapMemory(m_Allocator, memMap);

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
		transfer.dst = handle.image;
		transfer.finalLayout = finalLayout;
		transfer.memory = memMap;
		m_ImageCopies.push_back(transfer);
		m_BufferSet.insert(intImage);
	}
	m_ImageSet.insert(handle.image);
	return handle;
}

VkBufferHandle DeviceAllocator::AllocateBuffer(const VkBufferUsageFlags usage, uint64_t size, void * data, uint32_t memoryTypeBits) {
	VkBufferHandle ret;

	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.size = size;
	createInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo memReqs = {};
	memReqs.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	memReqs.preferredFlags = VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	memReqs.memoryTypeBits = memoryTypeBits;
	VkResult vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &ret.buffer, &ret.memory, nullptr);
	if (vkres != VK_SUCCESS) {
		printf("Error allocating memory for buffer\n");
		return ret;
	}
	m_BufferCounter++;
	if (data) {
		VmaAllocationInfo info;
		BufferTransfer transfer;
		memReqs.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		memReqs.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &transfer.src, &transfer.memory, &info);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate buffer\n");
			return ret;
		}

		void* ptr = nullptr;
		vmaMapMemory(m_Allocator, transfer.memory, &ptr);
		memcpy(ptr, data, size);
		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = nullptr;
		range.memory = info.deviceMemory;
		range.offset = info.offset;
		range.size = info.size;
		vkFlushMappedMemoryRanges(*m_Device, 1, &range);
		vmaUnmapMemory(m_Allocator, transfer.memory);

		transfer.dst = ret.buffer;
		transfer.copy.dstOffset = 0;
		transfer.copy.srcOffset = 0;
		transfer.copy.size = size;
		m_BufferCopies.push_back(transfer);
		m_BufferCounter++;
		m_BufferSet.insert(transfer.src);
	}
	m_BufferSet.insert(ret.buffer);
	return ret;
}

void DeviceAllocator::UpdateBuffer(VkBufferHandle& handle, uint64_t size, void* data) {
	if (data) {
		VmaAllocationInfo info;
		BufferTransfer transfer;
		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.size = size;
		createInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo memReqs = {};
		memReqs.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		memReqs.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		VkResult vkres = vmaCreateBuffer(m_Allocator, &createInfo, &memReqs, &transfer.src, &transfer.memory, &info);
		if (vkres != VK_SUCCESS) {
			printf("Error allocating memory for intermediate buffer\n");
			return;
		}
		void* ptr = nullptr;
		vmaMapMemory(m_Allocator, transfer.memory, &ptr);
		memcpy(ptr, data, size);

		VkMappedMemoryRange range = {};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.pNext = nullptr;
		range.memory = info.deviceMemory;
		range.offset = info.offset;
		range.size = info.size;
		vkFlushMappedMemoryRanges(*m_Device, 1, &range);
		vmaUnmapMemory(m_Allocator, transfer.memory);

		transfer.dst = handle.buffer;
		transfer.copy.dstOffset = 0;
		transfer.copy.srcOffset = 0;
		transfer.copy.size = size;
		m_BufferCopies.push_back(transfer);
		m_BufferCounter++;
		m_BufferSet.insert(transfer.src);
	}
}

void DeviceAllocator::DeAllocateBuffer(VkBufferHandle& handle) {
	vmaDestroyBuffer(m_Allocator, handle.buffer, handle.memory);
	m_BufferCounter--;
	m_BufferSet.erase(handle.buffer);
}

void DeviceAllocator::DeAllocateImage(VkImageHandle& handle) {
	vmaDestroyImage(m_Allocator, handle.image, handle.memory);
	m_ImageCounter--;
	m_ImageSet.erase(handle.image);
}

void DeviceAllocator::ScheduleTransfers(CommandBuffer* cmdBuffer) {
	uint32_t imageCount = (uint32_t)m_ImageCopies.size();
	if (imageCount > 0) {
		for (uint32_t i = 0; i < imageCount; ++i) {
			cmdBuffer->ImageBarrier(m_ImageCopies[i].dst, VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		}
		cmdBuffer->PushPipelineBarrier();
		for (uint32_t i = 0; i < imageCount; ++i) {
			vkCmdCopyBufferToImage(cmdBuffer->CmdBuffer(), m_ImageCopies[i].src, m_ImageCopies[i].dst, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_ImageCopies[i].copies.size(), &m_ImageCopies[i].copies[0]);
			cmdBuffer->ImageBarrier(m_ImageCopies[i].dst, VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, (VkImageLayout)m_ImageCopies[i].finalLayout);
		}
		cmdBuffer->PushPipelineBarrier();
		//m_ImageCopies.clear();
	}

	uint32_t bufferCopies = (uint32_t)m_BufferCopies.size();
	if (bufferCopies > 0) {
		for (uint32_t i = 0; i < bufferCopies; ++i) {
			vkCmdCopyBuffer(cmdBuffer->CmdBuffer(), m_BufferCopies[i].src, m_BufferCopies[i].dst, 1, &m_BufferCopies[i].copy);
		}
		//m_BufferCopies.clear();
	}
	vkCmdPipelineBarrier(cmdBuffer->CmdBuffer(), VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TRANSFER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 0, nullptr);
}

void DeviceAllocator::Clear() {
	for (auto& buffer : m_BufferCopies) {
		vmaDestroyBuffer(m_Allocator, buffer.src, buffer.memory);
		m_BufferSet.erase(buffer.src);
		m_BufferCounter--;
	}
	for (auto& buffer : m_ImageCopies) {
		vmaDestroyBuffer(m_Allocator, buffer.src, buffer.memory);
		m_BufferSet.erase(buffer.src);
		m_BufferCounter--;
	}
	m_BufferCopies.clear();
	m_ImageCopies.clear();
}

void DeviceAllocator::PrintStats() {

}