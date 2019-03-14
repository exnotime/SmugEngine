#pragma once
#include <string>
#include <glm/glm.hpp>
#include <AssetLoader/Resources.h>
#include "DeviceAllocator.h"
namespace smug {
class VkTexture {
  public:
	VkTexture();
	~VkTexture();

	void Init(const std::string& filename, DeviceAllocator* allocator, const VkDevice& device);
	void Init(const TextureInfo& texInfo, DeviceAllocator* allocator, const VkDevice& device);

	VkDescriptorImageInfo GetDescriptorInfo();
	VkImage GetImage() {
		return m_Image.image;
	}
	VkImageHandle GetImageHandle() {
		return m_Image;
	}

  private:
	int m_Width;
	int m_Height;
	int m_Channels;
	VkImageHandle m_Image;
	VkSampler m_Sampler;
	VkImageView m_View;
};
}