#include "ToneMapProgram.h"

ToneMapProgram::ToneMapProgram() {

}

ToneMapProgram::~ToneMapProgram() {

}

void ToneMapProgram::Init(vk::Device& device, const glm::vec2& screenSize, FrameBuffer& fbo, vk::DescriptorPool& descPool, vk::RenderPass& rp) {

	m_Pipeline.LoadPipelineFromFile(device, "shader/ToneMap.json", vk::Viewport(0,0, screenSize.x, screenSize.y), rp);
	//pepare desc sets
	vk::DescriptorSetAllocateInfo setAllocInfo;
	setAllocInfo.descriptorPool = descPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = m_Pipeline.GetDescriptorSetLayouts().data();

	for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
		m_DescSet[i] = device.allocateDescriptorSets(setAllocInfo)[0];
	}

	
	vk::SamplerCreateInfo sampInfo = {};
	sampInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	sampInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	sampInfo.anisotropyEnable = false;
	sampInfo.maxAnisotropy = 1.0f;
	sampInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
	sampInfo.compareEnable = false;
	sampInfo.compareOp = vk::CompareOp::eNever;
	sampInfo.magFilter = vk::Filter::eNearest;
	sampInfo.minFilter = vk::Filter::eNearest;
	sampInfo.maxLod = 1.0f;
	sampInfo.minLod = 0.0f;
	sampInfo.mipLodBias = 0.0f;
	sampInfo.mipmapMode = vk::SamplerMipmapMode::eNearest;
	sampInfo.unnormalizedCoordinates = false;
	m_Sampler = device.createSampler(sampInfo);

	vk::WriteDescriptorSet writeSet[BUFFER_COUNT];
	vk::DescriptorImageInfo imageInfo[BUFFER_COUNT];

	for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
		imageInfo[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		imageInfo[i].sampler = m_Sampler;
		imageInfo[i].imageView = fbo.GetView(0, i);

		writeSet[i].descriptorCount = 1;
		writeSet[i].descriptorType = vk::DescriptorType::eCombinedImageSampler;
		writeSet[i].dstArrayElement = 0;
		writeSet[i].dstBinding = 0;
		writeSet[i].pImageInfo = &imageInfo[i];
		writeSet[i].dstSet = m_DescSet[i];
	}
	
	device.updateDescriptorSets(BUFFER_COUNT, writeSet, 0, nullptr);
}

void ToneMapProgram::Render(VulkanCommandBuffer& cmdBuffer, uint32_t frameIndex) {
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipeline());
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipelineLayout(), 0, m_DescSet[frameIndex], nullptr);
	//no need for a vertex buffer
	cmdBuffer.draw(3, 1, 0, 0);
}