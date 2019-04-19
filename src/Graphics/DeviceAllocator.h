#pragma once
#include <EASTL/vector.h>
#include <set>
#include "VulkanContext.h"
#include "vk_mem_alloc.h"
namespace smug {
struct ImageTransfer {
	VkBuffer src;
	VkImage dst;
	eastl::vector<VkBufferImageCopy> copies;
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
	void Init(VkDevice& device, VkPhysicalDevice& physicalDevice);
	void DeInit();
	VkImageHandle AllocateImage(VkImageCreateInfo* createInfo, uint64_t size = 0, void* data = nullptr);
	VkBufferHandle AllocateBuffer(const VkBufferUsageFlags usage, uint64_t size = 0, void * data = nullptr, uint32_t memoryTypeBits = 0, uint64_t dataSize = 0);
	void UpdateBuffer(VkBufferHandle& handle, uint64_t size, void* data = nullptr, uint64_t offset = 0);
	void DeAllocateBuffer(VkBufferHandle& handle);
	void DeAllocateImage(VkImageHandle& handle);
	VkDeviceMemory GetMemory(const VmaAllocation& a);
	VkDeviceSize GetMemoryOffset(const VmaAllocation& a);
	void ScheduleTransfers(CommandBuffer* cmdBuffer);
	void Clear();
	void PrintStats();
  private:
	eastl::vector<ImageTransfer> m_ImageCopies;
	eastl::vector<BufferTransfer> m_BufferCopies;
	std::set<VkBuffer> m_BufferSet;
	std::set<VkImage> m_ImageSet;
	VmaAllocator m_Allocator;
	VkDevice* m_Device;

	uint32_t m_ImageCounter = 0;
	uint32_t m_BufferCounter = 0;
};
}