#pragma once
#include "vk_mem_alloc.h"
#include <vector>
#include "VulkanContext.h"
namespace smug {
struct ImageTransfer {
	VkBuffer src;
	VkImage dst;
	std::vector<vk::BufferImageCopy> copies;
	VkImageLayout finalLayout;
};

struct BufferTransfer {
	VkBuffer src;
	VkBuffer dst;
	VkBufferCopy copy;
};

struct VkBufferHandle {
	VkMappedMemoryRange memory;
	VkBuffer buffer;
};

class CommandBuffer;
class DeviceAllocator {
  public:
	DeviceAllocator();
	~DeviceAllocator();
	void Init(vk::Device& device, vk::PhysicalDevice& physicalDevice);
	void AllocateImage(VkImageCreateInfo* createInfo, VkImage* image, uint64_t size = 0, void* data = nullptr);
	VkBufferHandle AllocateBuffer(const VkBufferUsageFlags usage, uint64_t size = 0, void * data = nullptr);
	void UpdateBuffer(VkBufferHandle& handle, uint64_t size, void* data = nullptr);
	void ScheduleTransfers(CommandBuffer* cmdBuffer);
	void Clear();
	void PrintStats();
  private:
	std::vector<ImageTransfer> m_ImageCopies;
	std::vector<BufferTransfer> m_BufferCopies;
	VmaAllocator m_Allocator;
	vk::Device* m_Device;
};
}