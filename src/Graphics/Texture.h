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

	void Init(const std::string& filename, DeviceAllocator* allocator, const vk::Device& device);
	void Init(const TextureInfo& texInfo, DeviceAllocator* allocator, const vk::Device& device);

	vk::DescriptorImageInfo GetDescriptorInfo();
	vk::Image GetImage() {
		return m_Image.image;
	}

  private:
	int m_Width;
	int m_Height;
	int m_Channels;
	VkImageHandle m_Image;
	vk::Sampler m_Sampler;
	vk::ImageView m_View;
};
}