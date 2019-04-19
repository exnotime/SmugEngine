#pragma once
#include "volk.h"
#include <deque>
#include <EASTL/vector.h>
#define BUFFER_COUNT 2

namespace smug {

struct VulkanSwapChain {
	VkSurfaceKHR Surface;
	VkSwapchainKHR SwapChain;
	VkImage Images[BUFFER_COUNT];
	VkImageView ImageViews[BUFFER_COUNT];
	VkFramebuffer FrameBuffers[BUFFER_COUNT];
	bool SRGB;
	VkFormat Format;
};

struct VulkanContext {
	VkInstance Instance;
	VkDevice Device;
	VkPhysicalDevice PhysicalDevice;
	VkDescriptorPool DescriptorPool;
	uint32_t FrameIndex;
};


static VkAccessFlags LayoutToAccessMask(VkImageLayout layout) {
	switch (layout) {
	case VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VkAccessFlagBits::VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		return VkAccessFlagBits::VK_ACCESS_MEMORY_READ_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_SHADER_READ_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VkAccessFlagBits::VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_GENERAL:
		return VkAccessFlagBits::VK_ACCESS_SHADER_WRITE_BIT;
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_PREINITIALIZED:
		return VkAccessFlags();
		break;
	case VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED:
		return VkAccessFlags();
		break;
	}
	return VkAccessFlags();
}

static VkImageAspectFlags LayoutToAspectMask(VkImageLayout layout) {
	if (layout == VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || layout == VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ||
	        layout == VkImageLayout::VK_IMAGE_LAYOUT_GENERAL || layout == VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR || layout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
	        layout == VkImageLayout::VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		return VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
	}
	else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT | VkImageAspectFlagBits::VK_IMAGE_ASPECT_STENCIL_BIT;
	}else if (layout == VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL){
		return VkImageAspectFlagBits::VK_IMAGE_ASPECT_DEPTH_BIT;
	} else {
		return VkImageAspectFlagBits::VK_IMAGE_ASPECT_METADATA_BIT;
	}
	return VkImageAspectFlagBits::VK_IMAGE_ASPECT_METADATA_BIT;
}

class CommandBuffer {
  public:
	CommandBuffer() {

	}

	CommandBuffer(VkCommandBuffer buffer) {
		m_Buffer = buffer;
	}

	~CommandBuffer() {
		int i = 0;
	}

	void Begin(const VkFramebuffer& frameBuffer, const VkRenderPass& renderPass) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.framebuffer = frameBuffer;
		inheritanceInfo.renderPass = renderPass;
		beginInfo.pInheritanceInfo = &inheritanceInfo;
		beginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(m_Buffer, &beginInfo);
	}

	void ImageBarrier(const VkImage& img, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkImageMemoryBarrier imgBarrier = {};
		imgBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imgBarrier.pNext = nullptr;
		imgBarrier.image = img;
		imgBarrier.oldLayout = oldLayout;
		imgBarrier.newLayout = newLayout;
		imgBarrier.srcAccessMask = LayoutToAccessMask(oldLayout);
		imgBarrier.dstAccessMask = LayoutToAccessMask(newLayout);
		imgBarrier.subresourceRange.aspectMask = LayoutToAspectMask(newLayout);
		imgBarrier.subresourceRange.baseArrayLayer = 0;
		imgBarrier.subresourceRange.baseMipLevel = 0;
		imgBarrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
		imgBarrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
		m_ImgBarriers.push_back(imgBarrier);
	}

	void PushPipelineBarrier() {
		vkCmdPipelineBarrier(m_Buffer,VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, (uint32_t)m_ImgBarriers.size(), m_ImgBarriers.data());
		m_ImgBarriers.resize(0);
	}

	void Reset() {
		vkResetCommandBuffer( m_Buffer, VkCommandBufferResetFlagBits::VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		m_ImgBarriers.resize(0);
	}

	VkCommandBuffer CmdBuffer() { return m_Buffer; }

  private:
	eastl::vector<VkImageMemoryBarrier> m_ImgBarriers;
	VkCommandBuffer m_Buffer;
};

class VulkanCommandBufferFactory {
  public:
	VulkanCommandBufferFactory() {

	}
	~VulkanCommandBufferFactory() {
		
	}

	void Init(VkDevice device, int queueFamilyIndex, uint32_t bufferCount) {
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vkCreateCommandPool(device, &poolInfo,nullptr, &m_CmdPools[0]);
		vkCreateCommandPool(device, &poolInfo, nullptr, &m_CmdPools[1]);

		VkCommandBufferAllocateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.pNext = nullptr;
		bufferInfo.commandBufferCount = bufferCount;
		bufferInfo.commandPool = m_CmdPools[0];
		bufferInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		eastl::vector<VkCommandBuffer> buffers;
		buffers.resize(bufferCount);
		vkAllocateCommandBuffers(device, &bufferInfo, &buffers[0]);
		for (uint32_t i = 0; i < buffers.size(); ++i) {
			m_CommandBuffers.push_back(new CommandBuffer(buffers[i]));
		}
		m_ResetBuffers.insert(m_ResetBuffers.begin(), m_CommandBuffers.begin(), m_CommandBuffers.end());
	}

	void Shutdown() {
		for (auto cb : m_CommandBuffers) {
			delete cb;
		}
	}

	void Reset(VkDevice device, int frameIndex) {
		while (!m_UsedBuffers.empty()) {
			auto& buffer = m_UsedBuffers.front();
			buffer->Reset();
			m_UsedBuffers.pop_front();
			m_ResetBuffers.push_back(buffer);
		}
		vkResetCommandPool(device, m_CmdPools[frameIndex], VkCommandPoolResetFlagBits::VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}

	CommandBuffer* GetNextBuffer() {
		CommandBuffer* buffer = m_ResetBuffers.front();
		m_ResetBuffers.pop_front();
		return buffer;
	}

	void EndBuffer(CommandBuffer* buffer) {
		vkEndCommandBuffer(buffer->CmdBuffer());
		m_UsedBuffers.push_back(buffer);
	}

  private:
	eastl::vector<CommandBuffer*> m_CommandBuffers;
	std::deque<CommandBuffer*> m_ResetBuffers;
	std::deque<CommandBuffer*> m_UsedBuffers;
	VkCommandPool m_CmdPools[BUFFER_COUNT];
};

class DeviceQueue {
  public:
	DeviceQueue() {

	}
	~DeviceQueue() {

	}

	void Init(VkPhysicalDevice physDevice, VkQueueFlagBits type) {
		//find queue index for type
		m_QueueIndex = 0;
		uint32_t queueCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueCount, nullptr);
		eastl::vector<VkQueueFamilyProperties> props(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueCount, &props[0]);
		for (; m_QueueIndex < queueCount; ++m_QueueIndex) {
			if (props[m_QueueIndex].queueFlags & type) {
				break;
			}
		}
		m_QueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		m_QueueInfo.pNext = nullptr;
		m_QueueInfo.flags = 0;
		m_QueueInfo.queueCount = 1;
		m_QueueInfo.queueFamilyIndex = m_QueueIndex;
		m_QueuePrio = 0.0f;
		m_QueueInfo.pQueuePriorities = &m_QueuePrio;
	}

	void SetQueue(const VkDevice& device) {
		vkGetDeviceQueue(device, m_QueueIndex, 0, &m_Queue);
	}

	void Submit(const eastl::vector<VkCommandBuffer>& cmdBuffers, const eastl::vector<VkSemaphore> waitSemaphores,
	            const eastl::vector<VkSemaphore> signalSemaphores, VkFence fence) {
		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;
		submit.commandBufferCount = (uint32_t)cmdBuffers.size();
		submit.pCommandBuffers = cmdBuffers.data();
		submit.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
		submit.pSignalSemaphores = signalSemaphores.data();
		submit.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
		submit.pWaitSemaphores = waitSemaphores.data();

		eastl::vector<VkPipelineStageFlags> flags;
		for (auto& waits : waitSemaphores)
			flags.push_back(VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

		submit.pWaitDstStageMask = flags.data();
		vkQueueSubmit(m_Queue, 1, &submit, fence);
	}

	void Submit(VkCommandBuffer cmdBuffer, VkSemaphore waitSemaphore, VkSemaphore signalSemaphore, VkFence fence) {
		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;
		submit.signalSemaphoreCount = (signalSemaphore) ? 1 : 0;
		submit.pSignalSemaphores = &signalSemaphore;
		submit.waitSemaphoreCount = (waitSemaphore) ? 1 : 0;
		submit.pWaitSemaphores = &waitSemaphore;
		VkPipelineStageFlags flags = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submit.pWaitDstStageMask = &flags;

		vkQueueSubmit(m_Queue, 1, &submit, fence);
	}

	void Submit(VkCommandBuffer cmdBuffer) {
		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;
		VkPipelineStageFlags flags = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submit.pWaitDstStageMask = &flags;

		vkQueueSubmit(m_Queue, 1, &submit, nullptr);
	}

	void Submit(const eastl::vector<VkCommandBuffer>& buffers) {
		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;
		submit.commandBufferCount = (uint32_t)buffers.size();
		submit.pCommandBuffers = buffers.data();
		VkPipelineStageFlags flags = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		submit.pWaitDstStageMask = &flags;

		vkQueueSubmit(m_Queue, 1, &submit, nullptr);
	}

	VkDeviceQueueCreateInfo& GetInfo() {
		return m_QueueInfo;
	}
	int GetQueueIndex() {
		return m_QueueIndex;
	}

	VkQueue GetQueue() { return m_Queue; }

  private:
	uint32_t m_QueueIndex;
	float m_QueuePrio;
	VkDeviceQueueCreateInfo m_QueueInfo;
	VkQueue m_Queue;

};

}