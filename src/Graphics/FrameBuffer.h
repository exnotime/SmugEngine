#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <unordered_map>
#include "VulkanContext.h"
#include "vk_mem_alloc.h"
#include "GraphicsExport.h" 

namespace smug {

	struct RenderTarget {
		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t Name;
		vk::Format Format;
		vk::Image Handle;
		vk::ImageView View;
		vk::ImageLayout Layout;
		VmaAllocation Memory;
	};

	struct GFX_DLL SubPass {
		std::vector<uint32_t> RenderTargets;
		uint32_t DepthStencilAttachment = UINT_MAX;
	};

class GFX_DLL FrameBufferManager {
  public:
	FrameBufferManager();
	~FrameBufferManager();
	void Init(const vk::Device& device, const vk::PhysicalDevice& gpu, const glm::vec2& size, const std::vector<vk::Format>& formats, const std::vector<vk::ImageUsageFlags>& usages, uint32_t bufferCount);
	void Resize(const glm::vec2& size);
	void ChangeLayout(CommandBuffer& cmdBuffer, const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex);
	void SetLayouts(const std::vector<vk::ImageLayout>& newLayouts, uint32_t frameIndex);

	void AllocRenderTarget(uint32_t name, uint32_t width, uint32_t height, uint32_t depth, vk::Format format, vk::ImageLayout initialLayout);
	void CreateRenderPass(uint32_t name, std::vector<SubPass> subPasses);

	//Getters
	std::vector<vk::Image>& GetImages() {
		return m_Images;
	}
	std::vector<vk::ImageView>& GetViews() {
		return m_ImageViews;
	}
	std::vector<vk::Format>& GetFormats() {
		return m_Formats;
	}
	std::vector<vk::ImageLayout>& GetLayouts() {
		return m_CurrentLayouts;
	}
	std::vector<vk::DescriptorImageInfo>& GetDescriptors() {
		return m_Descriptors;
	}
	vk::RenderPass GetRenderPass() {
		return m_RenderPass;
	}
	vk::Framebuffer GetFrameBuffer(uint32_t frameIndex) {
		return m_FrameBuffers[frameIndex];
	}

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
	glm::vec2 GetSize() const {
		return m_FrameBufferSize;
	}


  private:
	vk::Device m_Device;
	VmaAllocator m_DeviceAllocator;
	std::unordered_map<uint32_t, RenderTarget> m_RenderTargets;
	std::unordered_map<uint32_t, vk::RenderPass> m_RenderPasses;
	std::unordered_map<uint32_t, vk::Framebuffer> m_FrameBuffersT;

	glm::vec2 m_FrameBufferSize;
	vk::Framebuffer m_FrameBuffers[BUFFER_COUNT];
	vk::RenderPass m_RenderPass;
	vk::DeviceMemory m_Memory;
	std::vector<vk::Image> m_Images;
	std::vector<vk::ImageView> m_ImageViews;
	std::vector<vk::Format> m_Formats;
	std::vector<vk::ImageLayout> m_CurrentLayouts;
	std::vector<vk::DescriptorImageInfo> m_Descriptors;
	uint64_t m_MemorySize;
	uint32_t m_FormatCount;
	uint32_t m_BufferCount;
};
}
