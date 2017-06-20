#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "VkMemory.h"
#include "VulkanContext.h"
class FrameBuffer {
public:
	FrameBuffer();
	~FrameBuffer();
	void Init(const vk::Device& device, const vk::PhysicalDevice& gpu, const glm::vec2& size,
		const std::vector<vk::Format>& formats, const std::vector<vk::ImageUsageFlags>& usages);
	void Resize(const glm::vec2& size);
	void ChangeLayout(VulkanCommandBuffer& cmdBuffer, const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex);
	void SetLayouts(const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex);
	//Getters
	std::vector<vk::Image>& GetImages() { return m_Images; }
	std::vector<vk::ImageView>& GetViews() { return m_ImageViews; }
	std::vector<vk::Format>& GetFormats() { return m_Formats; }
	std::vector<vk::ImageLayout>& GetLayouts() { return m_CurrentLayouts; }
	vk::RenderPass GetRenderPass() { return m_RenderPass; }
	vk::Framebuffer GetFrameBuffer(uint32_t frameIndex) { return m_FrameBuffers[frameIndex]; }

	vk::Image GetImage(uint32_t index, uint32_t frameIndex) {
		return m_Images[m_FormatCount * frameIndex + index]; 
	}
	vk::ImageView& GetView(uint32_t index, uint32_t frameIndex) {
		return m_ImageViews[m_FormatCount * frameIndex + index];
	}
	vk::Image GetImage(vk::Format f, uint32_t frameIndex) {
		uint32_t i = frameIndex * m_FormatCount;
		for (; i < m_FormatCount; ++i)
			if (m_Formats[i] == f)
				break;
		return m_Images[m_FormatCount * frameIndex + i];
	}
	vk::ImageView& GetView(vk::Format f, uint32_t frameIndex) {
		uint32_t i = frameIndex * m_FormatCount;
		for (; i < m_FormatCount; ++i)
			if (m_Formats[i] == f)
				break;
		return m_ImageViews[m_FormatCount * frameIndex + i];
	}
	glm::vec2 GetSize() const { return m_FrameBufferSize; }

	
private:
	glm::vec2 m_FrameBufferSize;
	VkMemory m_Memory;
	vk::Framebuffer m_FrameBuffers[BUFFER_COUNT];
	vk::RenderPass m_RenderPass;
	std::vector<vk::Image> m_Images;
	std::vector<vk::ImageView> m_ImageViews;
	std::vector<vk::Format> m_Formats;
	std::vector<vk::ImageLayout> m_CurrentLayouts;
	uint64_t m_MemorySize;
	uint32_t m_FormatCount;
};