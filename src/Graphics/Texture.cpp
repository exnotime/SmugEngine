#include "Texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"
#include <gli/gli.hpp>
#include "vk_mem_alloc.h"

using namespace smug;
VkTexture::VkTexture() {

}
VkTexture::~VkTexture() {

}

void VkTexture::Init(const std::string& filename, DeviceAllocator* allocator, const VkDevice& device) {
	gli::texture texture(gli::load(filename));
	//create image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	void* data = nullptr;
	uint64_t dataSize = 0;
	if (texture.target() == gli::TARGET_CUBE) {
		gli::texture_cube cubeTex(texture);
		imageInfo.arrayLayers = (uint32_t)cubeTex.faces();
		imageInfo.extent = { (uint32_t)cubeTex[0].extent().x, (uint32_t)cubeTex[0].extent().y, 1 };
		imageInfo.format = static_cast<VkFormat>(cubeTex.format());
		imageInfo.mipLevels = (uint32_t)cubeTex.levels();
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		data = cubeTex.data();
		dataSize = cubeTex.size();
	} else if (texture.target() == gli::TARGET_2D) {
		gli::texture2d tex2D(texture);
		imageInfo.arrayLayers = 1;
		imageInfo.extent = { (uint32_t)tex2D[0].extent().x, (uint32_t)tex2D[0].extent().y, 1 };
		imageInfo.format = static_cast<VkFormat>(tex2D.format());
		imageInfo.mipLevels = (uint32_t)tex2D.levels();
		data = tex2D.data();
		dataSize = tex2D.size();
	} else {
		printf("Error unsupported image type\n");
		return;
	}

	m_Image = allocator->AllocateImage(&imageInfo, dataSize, data);
	//create view
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = imageInfo.format;
	viewInfo.image = m_Image.image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (texture.target() == gli::TARGET_CUBE) {
		viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	} else if (texture.target() == gli::TARGET_2D) {
		viewInfo.subresourceRange.levelCount = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	vkCreateImageView(device, &viewInfo, nullptr, &m_View);
	//create sampler
	VkSamplerCreateInfo sampInfo = {};
	sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampInfo.pNext = nullptr;
	sampInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.anisotropyEnable = true;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
	sampInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
	sampInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
	sampInfo.maxLod = (float)texture.levels();
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampInfo.unnormalizedCoordinates = false;
	vkCreateSampler(device, &sampInfo, nullptr, &m_Sampler);
}

void VkTexture::Init(const TextureInfo& texInfo, DeviceAllocator* allocator, const VkDevice& device) {
	//create image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.arrayLayers = texInfo.Layers;
	imageInfo.extent = { texInfo.Width, texInfo.Height, 1 };
	imageInfo.format = (VkFormat)texInfo.Format;
	imageInfo.mipLevels = texInfo.MipCount;

	//allocate memory
	m_Image = allocator->AllocateImage(&imageInfo, texInfo.LinearSize, texInfo.Data);
	//create view
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = imageInfo.format;
	viewInfo.image = m_Image.image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (texInfo.Layers == 6) {
		viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	} else {
		viewInfo.subresourceRange.levelCount = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	vkCreateImageView(device, &viewInfo, nullptr, &m_View);
	//create sampler
	VkSamplerCreateInfo sampInfo = {};
	sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampInfo.pNext = nullptr;
	sampInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.anisotropyEnable = true;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
	sampInfo.magFilter = VkFilter::VK_FILTER_LINEAR;
	sampInfo.minFilter = VkFilter::VK_FILTER_LINEAR;
	sampInfo.maxLod = (float)texInfo.MipCount;
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampInfo.unnormalizedCoordinates = false;
	vkCreateSampler(device, &sampInfo, nullptr, &m_Sampler);
}


VkDescriptorImageInfo VkTexture::GetDescriptorInfo() {
	VkDescriptorImageInfo info;
	info.imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	info.imageView = m_View;
	info.sampler = m_Sampler;
	return info;
}