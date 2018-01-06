#pragma once
#include <vulkan/vulkan.hpp>
#include <deque>
#define BUFFER_COUNT 2

struct VulkanContext {
	vk::Instance Instance;
	vk::Device Device;
	vk::PhysicalDevice PhysicalDevice;
	uint32_t FrameIndex;
};

struct VulkanSwapChain {
	vk::SurfaceKHR Surface;
	vk::SwapchainKHR SwapChain;
	vk::Image Images[BUFFER_COUNT];
	vk::ImageView ImageViews[BUFFER_COUNT];
	vk::Framebuffer FrameBuffers[BUFFER_COUNT];
	bool SRGB;
	vk::Format Format;
};


static vk::AccessFlags LayoutToAccessMask(vk::ImageLayout layout) {
	switch (layout) {
	case vk::ImageLayout::eColorAttachmentOptimal:
		return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
		break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;
	case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
		return vk::AccessFlagBits::eShaderRead;
		break;
	case vk::ImageLayout::ePresentSrcKHR:
		return vk::AccessFlagBits::eMemoryRead;
		break;
	case vk::ImageLayout::eShaderReadOnlyOptimal:
		return vk::AccessFlagBits::eShaderRead;
		break;
	case vk::ImageLayout::eTransferDstOptimal:
		return vk::AccessFlagBits::eTransferWrite;
		break;
	case vk::ImageLayout::eTransferSrcOptimal:
		return vk::AccessFlagBits::eTransferRead;
		break;
	case vk::ImageLayout::eGeneral:
		return vk::AccessFlagBits::eShaderWrite;
		break;
	case vk::ImageLayout::ePreinitialized:
		return vk::AccessFlags();
		break;
	case vk::ImageLayout::eUndefined:
		return vk::AccessFlags();
		break;
	}
	return vk::AccessFlags();
}

static vk::ImageAspectFlags LayoutToAspectMask(vk::ImageLayout layout) {
	if (layout == vk::ImageLayout::eColorAttachmentOptimal || layout == vk::ImageLayout::eShaderReadOnlyOptimal ||
		layout == vk::ImageLayout::eGeneral || layout == vk::ImageLayout::ePresentSrcKHR || layout == vk::ImageLayout::eTransferDstOptimal ||
		layout == vk::ImageLayout::eTransferSrcOptimal) {
		return vk::ImageAspectFlagBits::eColor;
	}
	else if (layout == vk::ImageLayout::eDepthStencilAttachmentOptimal || layout == vk::ImageLayout::eDepthStencilReadOnlyOptimal) {
		return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
	}
	else {
		return vk::ImageAspectFlagBits::eMetadata;
	}
	return vk::ImageAspectFlagBits::eMetadata;
}

class VulkanCommandBuffer : public vk::CommandBuffer {
  public:
	VulkanCommandBuffer() {

	}

	VulkanCommandBuffer(vk::CommandBuffer buffer) {
		*static_cast<vk::CommandBuffer*>(this) = buffer;
	}

	~VulkanCommandBuffer() {

	}

	void Begin(const vk::Framebuffer& frameBuffer, const vk::RenderPass& renderPass) {
		vk::CommandBufferBeginInfo beginInfo;
		vk::CommandBufferInheritanceInfo inheritanceInfo;
		inheritanceInfo.framebuffer = frameBuffer;
		inheritanceInfo.renderPass = renderPass;
		beginInfo.pInheritanceInfo = &inheritanceInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

		this->begin(&beginInfo);
	}

	void ImageBarrier(const vk::Image& img, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
		vk::ImageMemoryBarrier imgBarrier;
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
		this->pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, vk::DependencyFlagBits::eByRegion,
		                      0, nullptr, 0, nullptr, (uint32_t)m_ImgBarriers.size(), m_ImgBarriers.data());
		m_ImgBarriers.clear();
	}

  private:
	std::vector<vk::ImageMemoryBarrier> m_ImgBarriers;
};

class VulkanCommandBufferFactory {
public:
	VulkanCommandBufferFactory() {

	}
	~VulkanCommandBufferFactory() {

	}

	void Init(vk::Device device, int queueFamilyIndex, uint32_t bufferCount) {
		vk::CommandPoolCreateInfo poolInfo;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		m_CmdPools[0] = device.createCommandPool(poolInfo);
		m_CmdPools[1] = device.createCommandPool(poolInfo);

		vk::CommandBufferAllocateInfo bufferInfo;
		bufferInfo.commandBufferCount = bufferCount;
		bufferInfo.commandPool = m_CmdPools[0];
		bufferInfo.level = vk::CommandBufferLevel::ePrimary;

		auto& buffers = device.allocateCommandBuffers(bufferInfo);
		for (uint32_t i = 0; i < buffers.size(); ++i) {
			m_CommandBuffers.push_back(VulkanCommandBuffer(buffers[i]));
		}
		m_ResetBuffers.insert(m_ResetBuffers.begin(), m_CommandBuffers.begin(), m_CommandBuffers.end());
	}

	void Reset(vk::Device device, int frameIndex) {
		while (!m_UsedBuffers.empty()) {
			auto& buffer = m_UsedBuffers.front();
			buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
			m_UsedBuffers.pop_front();
			m_ResetBuffers.push_back(buffer);
		}
		device.resetCommandPool(m_CmdPools[frameIndex], vk::CommandPoolResetFlagBits::eReleaseResources);
	}

	VulkanCommandBuffer& GetNextBuffer() {
		VulkanCommandBuffer& buffer = m_ResetBuffers.front();
		m_ResetBuffers.pop_front();
		return buffer;
	}

	void EndBuffer(VulkanCommandBuffer& buffer) {
		buffer.end();
		m_UsedBuffers.push_back(buffer);
	}

private:
	std::vector<VulkanCommandBuffer> m_CommandBuffers;
	std::deque<VulkanCommandBuffer> m_ResetBuffers;
	std::deque<VulkanCommandBuffer> m_UsedBuffers;
	vk::CommandPool m_CmdPools[BUFFER_COUNT];
};

class VulkanQueue : public vk::Queue {
  public:
	VulkanQueue() {

	}
	~VulkanQueue() {

	}

	void Init(const vk::PhysicalDevice& physDevice, vk::QueueFlagBits type) {
		//find queue index for type
		m_QueueIndex = 0;
		for (auto& queueProps : physDevice.getQueueFamilyProperties()) {
			if (queueProps.queueFlags & type) {
				break;
			}
			m_QueueIndex++;
		}
		m_QueueInfo.queueCount = 1;
		m_QueueInfo.queueFamilyIndex = m_QueueIndex;
		m_QueuePrio = 0.0f;
		m_QueueInfo.pQueuePriorities = &m_QueuePrio;
	}

	void Submit(const std::vector<vk::CommandBuffer>& cmdBuffers, const std::vector<vk::Semaphore> waitSemaphores,
	            const std::vector<vk::Semaphore> signalSemaphores, vk::Fence fence) {
		vk::SubmitInfo submit;
		submit.commandBufferCount = (uint32_t)cmdBuffers.size();
		submit.pCommandBuffers = cmdBuffers.data();
		submit.signalSemaphoreCount = (uint32_t)signalSemaphores.size();
		submit.pSignalSemaphores = signalSemaphores.data();
		submit.waitSemaphoreCount = (uint32_t)waitSemaphores.size();
		submit.pWaitSemaphores = waitSemaphores.data();

		std::vector<vk::PipelineStageFlags> flags;
		for(auto& waits : waitSemaphores)
			flags.push_back(vk::PipelineStageFlagBits::eBottomOfPipe);

		submit.pWaitDstStageMask = flags.data();

		this->submit(1, &submit, fence);
	}

	void Submit(const vk::CommandBuffer& cmdBuffer, const vk::Semaphore& waitSemaphore,
	            const vk::Semaphore& signalSemaphore, const vk::Fence fence) {
		vk::SubmitInfo submit;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;
		submit.signalSemaphoreCount = (signalSemaphore) ? 1 : 0;
		submit.pSignalSemaphores = &signalSemaphore;
		submit.waitSemaphoreCount = (waitSemaphore) ? 1 : 0;
		submit.pWaitSemaphores = &waitSemaphore;
		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eBottomOfPipe;
		submit.pWaitDstStageMask = &flags;

		this->submit(1, &submit, fence);
	}

	void Submit(const vk::CommandBuffer cmdBuffer) {
		vk::SubmitInfo submit;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmdBuffer;
		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eBottomOfPipe;
		submit.pWaitDstStageMask = &flags;

		this->submit(1, &submit, nullptr);
	}

	void Submit(const std::vector<vk::CommandBuffer> buffers) {
		vk::SubmitInfo submit;
		submit.commandBufferCount = (uint32_t)buffers.size();
		submit.pCommandBuffers = buffers.data();
		vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eBottomOfPipe;
		submit.pWaitDstStageMask = &flags;

		this->submit(1, &submit, nullptr);
	}

	vk::DeviceQueueCreateInfo& GetInfo() {
		return m_QueueInfo;
	}
	int GetQueueIndex() {
		return m_QueueIndex;
	}

  private:
	int m_QueueIndex;
	float m_QueuePrio;
	vk::DeviceQueueCreateInfo m_QueueInfo;

};