#include "LightingProgram.h"
#include "ResourceHandler.h"
#include <AssetLoader/AssetLoader.h>
#include <Utility/Hash.h>

using namespace smug;

LightingProgram::LightingProgram() {

}

LightingProgram::~LightingProgram() {

}

struct PerFrame {
	glm::vec4 LightDir;
	glm::mat4 InvViewProj;
	glm::vec4 CamPos;
	glm::vec2 ScreenSize;
};


void LightingProgram::Init(VkDevice device, VkDescriptorPool descPool, ResourceHandler* resources, FrameBufferManager* framebuffers, DeviceAllocator* allocator) {
	m_UniformBuffer = allocator->AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(PerFrame));

	m_PipelineState = g_AssetLoader.LoadAsset("assets/shaders/Lighting.shader");
	const PipelineState& ps = resources->GetPipelineState(m_PipelineState);
	const auto& descs = ps.GetDescriptorSetLayouts();

	VkDescriptorSetAllocateInfo descAllocInfo = {};
	descAllocInfo.descriptorPool = descPool;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = &descs[0];
	descAllocInfo.pNext = nullptr;
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkAllocateDescriptorSets(device, &descAllocInfo, &m_DescSet);

	VkSamplerCreateInfo sampInfo = {};
	sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampInfo.pNext = nullptr;
	sampInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
	sampInfo.anisotropyEnable = false;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = VkBorderColor::VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = VkCompareOp::VK_COMPARE_OP_NEVER;
	sampInfo.magFilter = VkFilter::VK_FILTER_NEAREST;
	sampInfo.minFilter = VkFilter::VK_FILTER_NEAREST;
	sampInfo.maxLod = 1.0f;
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = VkSamplerMipmapMode::VK_SAMPLER_MIPMAP_MODE_NEAREST;
	sampInfo.unnormalizedCoordinates = false;
	vkCreateSampler(device, &sampInfo, nullptr, &m_Sampler);

	eastl::vector<VkWriteDescriptorSet> writes;
	eastl::vector<VkDescriptorImageInfo> images;
	images.reserve(6);
	VkDescriptorImageInfo i = {};
	i.sampler = m_Sampler;
	uint32_t b = 0;
	VkWriteDescriptorSet w = {};
	w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	w.pNext = nullptr;
	w.dstArrayElement = 0;
	w.dstSet = m_DescSet;
	//output
	w.descriptorCount = 1;
	w.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	w.dstBinding = b++;
	i.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	i.imageView = framebuffers->GetRenderTarget(HashString("HDR")).View;
	images.push_back(i);
	w.pImageInfo = &images[0];
	writes.push_back(w);
	//Albedo
	w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	w.dstBinding = b++;
	i.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	i.imageView = framebuffers->GetRenderTarget(HashString("Albedo")).View;
	images.push_back(i);
	w.pImageInfo = &images[1];
	writes.push_back(w);
	//Depth
	w.dstBinding = b++;
	i.imageView = framebuffers->GetRenderTarget(HashString("DepthStencil")).View;
	images.push_back(i);
	w.pImageInfo = &images[2];
	writes.push_back(w);
	//Normals
	w.dstBinding = b++;
	i.imageView = framebuffers->GetRenderTarget(HashString("Normals")).View;
	images.push_back(i);
	w.pImageInfo = &images[3];
	writes.push_back(w);
	//Material
	w.dstBinding = b++;
	i.imageView = framebuffers->GetRenderTarget(HashString("Material")).View;
	images.push_back(i);
	w.pImageInfo = &images[4];
	writes.push_back(w);
	//Shadows
	w.dstBinding = b++;
	i.imageView = framebuffers->GetRenderTarget(HashString("Shadows")).View;
	images.push_back(i);
	w.pImageInfo = &images[5];
	writes.push_back(w);
	//uniform buffer
	w.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	w.dstBinding = b++;
	VkDescriptorBufferInfo buffer = {};
	buffer.offset = 0;
	buffer.range = VK_WHOLE_SIZE;
	buffer.buffer = m_UniformBuffer.buffer;
	w.pBufferInfo = &buffer;
	w.pImageInfo = nullptr;
	writes.push_back(w);

	vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
}

void LightingProgram::DeInit() {
}

void LightingProgram::Update(DeviceAllocator* allocator, const glm::vec3& lightDir, const glm::mat4& invviewProj, const glm::vec3& camPos, const glm::vec2& screenSize) {
	PerFrame pf;
	pf.CamPos = glm::vec4(camPos, 1);
	pf.InvViewProj = invviewProj;
	pf.LightDir = glm::vec4(lightDir, 1);
	pf.ScreenSize = screenSize;
	allocator->UpdateBuffer(m_UniformBuffer, sizeof(PerFrame), &pf);
}

void LightingProgram::Render(VkCommandBuffer cmdBuffer, ResourceHandler* resources, VkDescriptorSet iblSet, const glm::vec2& screenSize) {
	const PipelineState& ps = resources->GetPipelineState(m_PipelineState);
	vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, ps.GetPipeline());
	VkDescriptorSet sets[] = { m_DescSet, iblSet };
	vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, ps.GetPipelineLayout(), 0, 2, sets, 0, nullptr);
	vkCmdDispatch(cmdBuffer, screenSize.x + 7 / 8, screenSize.y + 7 / 8, 1);

}