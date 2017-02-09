#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanContext.h"
#include <gli/gli.hpp>
#define DEFAULT_DEVICE_SIZE 256 * 1024 * 1024
#define DEFAULT_STAGING_SIZE 64 * 1024 * 1024

struct Buffer {
	vk::Buffer BufferHandle;
	uint64_t BufferOffset;

	Buffer() {
		BufferHandle = nullptr;
		BufferOffset = 0;
	}
};

struct TextureTransfer {
	vk::Image Image;
	std::vector<vk::BufferImageCopy> copies;
};

class Memory {
public:
	Memory();
	~Memory();

	void Init(const vk::Device& device,const vk::PhysicalDevice& physDev, uint64_t deviceSize = DEFAULT_DEVICE_SIZE, uint64_t stageSize = DEFAULT_STAGING_SIZE);
	Buffer AllocateBuffer(uint64_t size, vk::BufferUsageFlagBits usage, void* data = nullptr);
	void AllocateImage(vk::Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t mipsCount, vk::ImageUsageFlagBits usage);
	void AllocateImage(vk::Image img, gli::texture2d* texture = nullptr, void* data = nullptr);
	void ScheduleTransfers(VulkanCommandBuffer cmdBuffer);
	void UpdateBuffer(Buffer buffer, uint64_t size, void* data);

private:
	vk::Device m_Device;
	vk::PhysicalDeviceMemoryProperties m_MemProps;

	vk::Buffer m_DeviceBuffer;
	vk::DeviceMemory m_DevMem;
	uint64_t m_DeviceSize;
	uint64_t m_DeviceOffset;

	uint64_t m_DeviceFragSpace; //keep track of fragmented space in case of cleanup in the future

	vk::Buffer m_StagingBuffer;
	vk::DeviceMemory m_StagingMem;
	uint64_t m_StagingSize;
	uint64_t m_StagingOffset;
	std::vector<vk::BufferCopy> m_Transfers;
	std::vector<TextureTransfer> m_ImageTransfers;
};