#pragma once
#include <vector>
#include <set>
#include "VulkanContext.h"
#include "vk_mem_alloc.h"
namespace smug {
struct ImageTransfer {
	VkBuffer src;
	VkImage dst;
	std::vector<vk::BufferImageCopy> copies;
	VkImageLayout finalLayout;
	VmaAllocation memory;
};

struct BufferTransfer {
	VkBuffer src;
	VkBuffer dst;
	VkBufferCopy copy;
	VmaAllocation memory;
};

struct VkBufferHandle {
	VmaAllocation memory;
	VkBuffer buffer;
};

struct VkImageHandle {
	VkImage image;
	VmaAllocation memory;
};

class CommandBuffer;
class DeviceAllocator {
  public:
	DeviceAllocator();
	~DeviceAllocator();
	void Init(vk::Device& device, vk::PhysicalDevice& physicalDevice);
	VkImageHandle AllocateImage(VkImageCreateInfo* createInfo, uint64_t size = 0, void* data = nullptr);
	VkBufferHandle AllocateBuffer(const VkBufferUsageFlags usage, uint64_t size = 0, void * data = nullptr);
	void UpdateBuffer(VkBufferHandle& handle, uint64_t size, void* data = nullptr);
	void DeAllocateBuffer(VkBufferHandle& handle);
	void DeAllocateImage(VkImageHandle& handle);
	void ScheduleTransfers(CommandBuffer* cmdBuffer);
	void Clear();
	void PrintStats();
  private:
	std::vector<ImageTransfer> m_ImageCopies;
	std::vector<BufferTransfer> m_BufferCopies;
	std::set<VkBuffer> m_BufferSet;
	std::set<VkImage> m_ImageSet;
	VmaAllocator m_Allocator;
	vk::Device* m_Device;

	uint32_t m_ImageCounter = 0;
	uint32_t m_BufferCounter = 0;
};
}