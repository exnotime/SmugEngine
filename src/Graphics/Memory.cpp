#include "Memory.h"

Memory::Memory() {

}

Memory::~Memory() {

}

void Memory::Init(const vk::Device& device, const vk::PhysicalDevice& physDev, uint64_t deviceSize, uint64_t stageSize) {
	m_MemProps = physDev.getMemoryProperties();
	m_Device = device;

	//allocate device memory
	vk::BufferCreateInfo devBufferInfo;
	devBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	devBufferInfo.size = deviceSize;
	devBufferInfo.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eStorageBuffer |
		vk::BufferUsageFlagBits::eStorageTexelBuffer | vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eVertexBuffer | 
		vk::BufferUsageFlagBits::eUniformTexelBuffer;
	m_DeviceBuffer = device.createBuffer(devBufferInfo);
	vk::MemoryRequirements memReq = device.getBufferMemoryRequirements(m_DeviceBuffer);

	vk::MemoryPropertyFlagBits deviceMemFlags = vk::MemoryPropertyFlagBits::eDeviceLocal;

	for (uint32_t i = 0; i < m_MemProps.memoryTypeCount; i++) {
		if ((m_MemProps.memoryTypes[i].propertyFlags & deviceMemFlags) == deviceMemFlags &&
			memReq.memoryTypeBits & (1 << i)) {
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = deviceSize;
			allocInfo.memoryTypeIndex = i;
			m_DevMem = device.allocateMemory(allocInfo);
			m_DeviceSize = deviceSize;
			break;
		}
	}

	vk::BufferCreateInfo stageBufferInfo;
	stageBufferInfo.sharingMode = vk::SharingMode::eExclusive;
	stageBufferInfo.size = stageSize;
	stageBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
	m_StagingBuffer = device.createBuffer(stageBufferInfo);
	vk::MemoryRequirements stageMemReq = device.getBufferMemoryRequirements(m_StagingBuffer);
	vk::MemoryPropertyFlags stagingMemFlags = vk::MemoryPropertyFlagBits::eHostVisible;

	for (uint32_t i = 0; i < m_MemProps.memoryTypeCount; i++) {
		if ((m_MemProps.memoryTypes[i].propertyFlags & stagingMemFlags) == stagingMemFlags &&
			stageMemReq.memoryTypeBits & (1 << i)) {
			vk::MemoryAllocateInfo allocInfo;
			allocInfo.allocationSize = stageSize;
			allocInfo.memoryTypeIndex = i;
			m_StagingMem = device.allocateMemory(allocInfo);
			m_StagingSize = stageSize;
			break;
		}
	}
	device.bindBufferMemory(m_DeviceBuffer, m_DevMem, 0);
	device.bindBufferMemory(m_StagingBuffer, m_StagingMem, 0);
	m_DeviceOffset = 0;
	m_StagingOffset = 0;
}

Buffer Memory::AllocateBuffer(uint64_t size, vk::BufferUsageFlagBits usage, void* data) {
	if (m_DeviceOffset + size > m_DeviceSize) {
		return Buffer();
	}

	vk::BufferCreateInfo bufferInfo;
	bufferInfo.sharingMode = vk::SharingMode::eExclusive;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	vk::Buffer buffer = m_Device.createBuffer(bufferInfo);
	
	auto memReq = m_Device.getBufferMemoryRequirements(buffer);
	m_DeviceOffset = (m_DeviceOffset + memReq.alignment - 1) & ~(memReq.alignment - 1);
	//bind buffer to memory
	m_Device.bindBufferMemory(buffer, m_DevMem, m_DeviceOffset);
	
	//if there is data allocate space in staging buffer
	if (data && m_StagingOffset + size < m_StagingSize) {
		//transfer data to staging buffer
		void* bufferPtr = m_Device.mapMemory(m_StagingMem, m_StagingOffset, size);
		memcpy(bufferPtr, data, size);
		vk::MappedMemoryRange range;
		range.memory = m_StagingMem;
		range.offset = m_StagingOffset;
		range.size = size;
		m_Device.flushMappedMemoryRanges(1, &range);
		m_Device.unmapMemory(m_StagingMem);

		vk::BufferCopy copy;
		copy.dstOffset = m_DeviceOffset;
		copy.srcOffset = m_StagingOffset;
		copy.size = size;
		m_Transfers.push_back(copy);
		m_StagingOffset += size;
	}

	Buffer buf;
	buf.BufferHandle = buffer;
	buf.BufferOffset = m_DeviceOffset;

	m_DeviceOffset += size;

	return buf;
}

void Memory::AllocateImage(vk::Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipsCount, vk::ImageUsageFlagBits usage) {
	//TODO: fill in

}

void Memory::AllocateImage(vk::Image img, gli::texture2d* texture, void* data) {
	vk::MemoryRequirements memReq = m_Device.getImageMemoryRequirements(img);
	m_DeviceOffset = (m_DeviceOffset + memReq.alignment) & ~memReq.alignment;

	if (m_DeviceOffset + memReq.size > m_DeviceSize) {
		return;
	}
	VkDeviceSize size = (texture) ? texture->size() : memReq.size;
	m_Device.bindImageMemory(img, m_DevMem, m_DeviceOffset);

	if (data && m_StagingOffset + size < m_StagingSize) {
		//transfer data to staging buffer
		void* bufferPtr = m_Device.mapMemory(m_StagingMem, m_StagingOffset, size);
		memcpy(bufferPtr, data, size);
		vk::MappedMemoryRange range;
		range.memory = m_StagingMem;
		range.offset = m_StagingOffset;
		range.size = size;
		m_Device.flushMappedMemoryRanges(1, &range);
		m_Device.unmapMemory(m_StagingMem);
		
		if (texture) {
			TextureTransfer transfer;
			transfer.Image = img;
			for (int i = 0; i < texture->levels(); i++) {
				vk::BufferImageCopy copy;
				gli::image mip = (*texture)[i];
				copy.bufferOffset = m_StagingOffset;
				copy.imageExtent = vk::Extent3D(mip.extent().x, mip.extent().y, 1);
				copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
				copy.imageSubresource.baseArrayLayer = 0;
				copy.imageSubresource.layerCount = 1;
				copy.imageSubresource.mipLevel = i;
				transfer.copies.push_back(copy);
				m_StagingOffset += mip.size();
			}
			m_ImageTransfers.push_back(transfer);
		} else {
			vk::BufferCopy copy;
			copy.dstOffset = m_DeviceOffset;
			copy.srcOffset = m_StagingOffset;
			copy.size = memReq.size;
			m_Transfers.push_back(copy);
			m_StagingOffset += memReq.size;
		}
		
	}
	m_DeviceOffset += memReq.size;
}

void Memory::AllocateImageCube(vk::Image img, gli::texture_cube* texture, void* data) {
	vk::MemoryRequirements memReq = m_Device.getImageMemoryRequirements(img);
	m_DeviceOffset = (m_DeviceOffset + memReq.alignment) & ~memReq.alignment;

	if (m_DeviceOffset + memReq.size > m_DeviceSize) {
		return;
	}
	VkDeviceSize size = (texture) ? texture->size() : memReq.size;
	m_Device.bindImageMemory(img, m_DevMem, m_DeviceOffset);

	if (data && m_StagingOffset + size < m_StagingSize) {
		//transfer data to staging buffer
		void* bufferPtr = m_Device.mapMemory(m_StagingMem, m_StagingOffset, size);
		memcpy(bufferPtr, data, size);
		vk::MappedMemoryRange range;
		range.memory = m_StagingMem;
		range.offset = m_StagingOffset;
		range.size = size;
		m_Device.flushMappedMemoryRanges(1, &range);
		m_Device.unmapMemory(m_StagingMem);

		if (texture) {
			TextureTransfer transfer;
			transfer.Image = img;
			for (int f = 0; f < texture->faces(); f++) {
				for (int i = 0; i < texture->levels(); i++) {
					vk::BufferImageCopy copy;
					gli::image mip = (*texture)[f][i];
					copy.bufferOffset = m_StagingOffset;
					copy.imageExtent = vk::Extent3D(mip.extent().x, mip.extent().y, 1);
					copy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
					copy.imageSubresource.baseArrayLayer = f;
					copy.imageSubresource.layerCount = 1;
					copy.imageSubresource.mipLevel = i;
					transfer.copies.push_back(copy);
					m_StagingOffset += mip.size();
				}
			}
			m_ImageTransfers.push_back(transfer);
		}
		else {
			vk::BufferCopy copy;
			copy.dstOffset = m_DeviceOffset;
			copy.srcOffset = m_StagingOffset;
			copy.size = memReq.size;
			m_Transfers.push_back(copy);
			m_StagingOffset += memReq.size;
		}

	}
	m_DeviceOffset += memReq.size;
}

void Memory::ScheduleTransfers(VulkanCommandBuffer cmdBuffer) {
	if (!m_Transfers.empty()) {
		//transfer data from staging buffer to device buffer
		cmdBuffer.copyBuffer(m_StagingBuffer, m_DeviceBuffer, m_Transfers);
	}
	if (!m_ImageTransfers.empty()) {
		//set image layout for textures to transfer destination
		for (auto& t : m_ImageTransfers) {
			cmdBuffer.ImageBarrier(t.Image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		}
		cmdBuffer.PushPipelineBarrier();
		//push copies
		for (auto& t : m_ImageTransfers) {
			cmdBuffer.copyBufferToImage(m_StagingBuffer, t.Image, vk::ImageLayout::eTransferDstOptimal, t.copies);
		}
		//set image layout for textures to shader read
		for (auto& t : m_ImageTransfers) {
			cmdBuffer.ImageBarrier(t.Image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		}
		cmdBuffer.PushPipelineBarrier();
	}
	//nothing can do work until all transfers are finished
	cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTopOfPipe, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, nullptr);
	//reset the stack for the staging buffer
	m_StagingOffset = 0;
	m_Transfers.clear();
}

void Memory::UpdateBuffer(Buffer buffer, uint64_t size, void* data) {
	if (data && m_StagingOffset + size < m_StagingSize) {
		//transfer data to stageing buffer
		void* bufferPtr = m_Device.mapMemory(m_StagingMem, m_StagingOffset, size);
		memcpy(bufferPtr, data, size);
		vk::MappedMemoryRange range;
		range.memory = m_StagingMem;
		range.offset = m_StagingOffset;
		range.size = size;
		m_Device.flushMappedMemoryRanges(1, &range);
		m_Device.unmapMemory(m_StagingMem);

		vk::BufferCopy copy;
		copy.dstOffset = buffer.BufferOffset;
		copy.srcOffset = m_StagingOffset;
		copy.size = size;
		m_Transfers.push_back(copy);
		m_StagingOffset += size;
	}
}