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

void VkTexture::Init(const std::string& filename, DeviceAllocator* allocator, const vk::Device& device) {
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
		imageInfo.extent = vk::Extent3D(cubeTex[0].extent().x, cubeTex[0].extent().y, 1);
		imageInfo.format = static_cast<VkFormat>(cubeTex.format());
		imageInfo.mipLevels = (uint32_t)cubeTex.levels();
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		data = cubeTex.data();
		dataSize = cubeTex.size();
	} else if (texture.target() == gli::TARGET_2D) {
		gli::texture2d tex2D(texture);
		imageInfo.arrayLayers = 1;
		imageInfo.extent = vk::Extent3D(tex2D[0].extent().x, tex2D[0].extent().y, 1 );
		imageInfo.format = static_cast<VkFormat>(tex2D.format());
		imageInfo.mipLevels = (uint32_t)tex2D.levels();
		data = tex2D.data();
		dataSize = tex2D.size();
	}
	else {
		printf("Error unsupported image type\n");
		return;
	}

	allocator->AllocateImage(&imageInfo, &m_Image, dataSize, data);
	//create view
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = imageInfo.format;
	viewInfo.image = m_Image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (texture.target() == gli::TARGET_CUBE) {
		viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	else if (texture.target() == gli::TARGET_2D) {
		viewInfo.subresourceRange.levelCount = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	m_View = device.createImageView(viewInfo);
	//create sampler
	vk::SamplerCreateInfo sampInfo;
	sampInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	sampInfo.anisotropyEnable = true;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = vk::CompareOp::eNever;
	sampInfo.magFilter = vk::Filter::eLinear;
	sampInfo.minFilter = vk::Filter::eLinear;
	sampInfo.maxLod = (float)texture.levels();
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	sampInfo.unnormalizedCoordinates = false;
	m_Sampler = device.createSampler(sampInfo);
}

void VkTexture::Init(const TextureInfo& texInfo, DeviceAllocator* allocator, const vk::Device& device) {
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
	imageInfo.extent = vk::Extent3D(texInfo.Width, texInfo.Height, 1);
	imageInfo.format = (VkFormat)texInfo.Format;
	imageInfo.mipLevels = texInfo.MipCount;

	//allocate memory
	allocator->AllocateImage(&imageInfo, &m_Image, texInfo.LinearSize, texInfo.Data);
	//create view
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.pNext = nullptr;
	viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	viewInfo.format = imageInfo.format;
	viewInfo.image = m_Image;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.layerCount = 1;

	if (texInfo.Layers == 6) {
		viewInfo.subresourceRange.layerCount = imageInfo.arrayLayers;
		viewInfo.subresourceRange.levelCount = imageInfo.mipLevels;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	else{
		viewInfo.subresourceRange.levelCount = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	m_View = device.createImageView(viewInfo);
	//create sampler
	vk::SamplerCreateInfo sampInfo;
	sampInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	sampInfo.anisotropyEnable = true;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = vk::CompareOp::eNever;
	sampInfo.magFilter = vk::Filter::eLinear;
	sampInfo.minFilter = vk::Filter::eLinear;
	sampInfo.maxLod = (float)texInfo.MipCount;
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	sampInfo.unnormalizedCoordinates = false;
	m_Sampler = device.createSampler(sampInfo);
}


vk::DescriptorImageInfo VkTexture::GetDescriptorInfo() {
	vk::DescriptorImageInfo info;
	info.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	info.imageView = m_View;
	info.sampler = m_Sampler;
	return info;
}