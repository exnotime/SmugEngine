#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanContext.h"
#include <gli/gli.hpp>
#include <AssetLoader/Resources.h>

#define MEGA_BYTE 1024 * 1024
#define DEFAULT_DEVICE_SIZE 256 * MEGA_BYTE
#define DEFAULT_STAGING_SIZE 64 * MEGA_BYTE

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
	Buffer AllocateBuffer(uint64_t size, vk::BufferUsageFlags usage, void* data = nullptr);
	void AllocateImage(vk::Image img, gli::texture2d* texture = nullptr, void* data = nullptr);
	void AllocateImage(vk::Image img, const TextureInfo& texInfo);
	void AllocateImageCube(vk::Image img, gli::texture_cube* texture = nullptr, void* data = nullptr);
	void AllocateImageCube(vk::Image img, const TextureInfo& texInfo);
	void ScheduleTransfers(VulkanCommandBuffer& cmdBuffer);
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