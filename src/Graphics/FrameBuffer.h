#pragma once
#include "volk.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <unordered_set>
#include "VulkanContext.h"
#include "vk_mem_alloc.h"
#include "GraphicsExport.h" 

namespace smug {

	struct RenderTarget {
		uint32_t Width;
		uint32_t Height;
		uint32_t Depth;
		uint32_t Name;
		VkFormat Format;
		VkImage Handle;
		VkImageView View;
		VkImageLayout Layout;
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
	void Init(const VkDevice& device, const VkPhysicalDevice& gpu, const glm::vec2& size, const std::vector<VkFormat>& formats, const std::vector<VkImageUsageFlags>& usages);
	void Resize(const glm::vec2& size);
	void ChangeLayout(CommandBuffer& cmdBuffer, const std::vector<VkImageLayout>& newLayouts);
	void SetLayouts(const std::vector<VkImageLayout>& newLayouts);

	void AllocRenderTarget(uint32_t name, uint32_t width, uint32_t height, uint32_t depth, VkFormat format, VkImageLayout initialLayout);
	VkRenderPass CreateRenderPass(uint32_t name, std::vector<SubPass> subPasses);

	//Getters
	std::vector<VkImage>& GetImages() {
		return m_Images;
	}
	std::vector<VkImageView>& GetViews() {
		return m_ImageViews;
	}
	std::vector<VkFormat>& GetFormats() {
		return m_Formats;
	}
	std::vector<VkImageLayout>& GetLayouts() {
		return m_CurrentLayouts;
	}
	std::vector<VkDescriptorImageInfo>& GetDescriptors() {
		return m_Descriptors;
	}
	VkRenderPass GetRenderPass() {
		return m_RenderPass;
	}
	VkFramebuffer GetFrameBuffer() {
		return m_FrameBuffer;
	}

	VkImage GetImage(uint32_t index, uint32_t frameIndex) {
		return m_Images[m_FormatCount * frameIndex + index];
	}
	VkImageView& GetView(uint32_t index, uint32_t frameIndex) {
		return m_ImageViews[m_FormatCount * frameIndex + index];
	}
	VkImage GetImage(VkFormat f, uint32_t frameIndex) {
		uint32_t i = frameIndex * m_FormatCount;
		for (; i < m_FormatCount; ++i)
			if (m_Formats[i] == f)
				break;
		return m_Images[m_FormatCount * frameIndex + i];
	}
	VkImageView& GetView(VkFormat f, uint32_t frameIndex) {
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
	VkDevice m_Device;
	VmaAllocator m_DeviceAllocator;
	std::unordered_map<uint32_t, RenderTarget> m_RenderTargets;
	std::unordered_map<uint32_t, VkRenderPass> m_RenderPasses;
	std::unordered_map<uint32_t, VkFramebuffer> m_FrameBuffersT;

	glm::vec2 m_FrameBufferSize;
	VkFramebuffer m_FrameBuffer;
	VkRenderPass m_RenderPass;
	VkDeviceMemory m_Memory;
	std::vector<VkImage> m_Images;
	std::vector<VkImageView> m_ImageViews;
	std::vector<VkFormat> m_Formats;
	std::vector<VkImageLayout> m_CurrentLayouts;
	std::vector<VkDescriptorImageInfo> m_Descriptors;
	uint64_t m_MemorySize;
	uint32_t m_FormatCount;
};
}
