#include "ToneMapProgram.h"
#include <Imgui/imgui.h>

using namespace smug;
ToneMapProgram::ToneMapProgram() {

}

ToneMapProgram::~ToneMapProgram() {

}

void ToneMapProgram::Init(vk::Device& device, const glm::vec2& screenSize, FrameBuffer& fbo, vk::DescriptorPool& descPool, vk::RenderPass& rp, DeviceAllocator& allocator) {

	m_Pipeline.LoadPipelineFromFile(device, "shader/ToneMap.json", rp);
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

	//each framebuffer texture + uniform buffer
	vk::WriteDescriptorSet writeSet[BUFFER_COUNT + 2];
	vk::DescriptorImageInfo imageInfo[BUFFER_COUNT];
	vk::DescriptorBufferInfo bufferInfo;

	//allocate memory for buffer
	m_Buffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ToneMapUniformData));
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;
	bufferInfo.buffer = m_Buffer.buffer;

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

	for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
		writeSet[BUFFER_COUNT + i].descriptorCount = 1;
		writeSet[BUFFER_COUNT + i].descriptorType = vk::DescriptorType::eUniformBuffer;
		writeSet[BUFFER_COUNT + i].dstArrayElement = 0;
		writeSet[BUFFER_COUNT + i].dstBinding = 1;
		writeSet[BUFFER_COUNT + i].dstSet = m_DescSet[i];
		writeSet[BUFFER_COUNT + i].pBufferInfo = &bufferInfo;
	}

	device.updateDescriptorSets(BUFFER_COUNT + 2, writeSet, 0, nullptr);
}

void ToneMapProgram::Update(DeviceAllocator& allocator) {
	static ToneMapUniformData data = {1.0f, 1.0f / 1.6f};

	//ImGui::Begin("Scene");
	//ImGui::Spacing();
	//ImGui::SliderFloat("Bright", &data.bright, 0.0f, 1.0f);
	//ImGui::SliderFloat("Exposure", &data.exposure, 0.0f, 1.0f);
	//ImGui::End();
	allocator.UpdateBuffer(m_Buffer, sizeof(ToneMapUniformData), &data);
}

void ToneMapProgram::Render(CommandBuffer& cmdBuffer, vk::Viewport viewport, uint32_t frameIndex) {
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipeline());
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipelineLayout(), 0, m_DescSet[frameIndex], nullptr);
	cmdBuffer.setViewport(0, 1, &viewport);
	//no need for a vertex buffer
	cmdBuffer.draw(3, 1, 0, 0);
}