#include "ToneMapProgram.h"
#include "Vertex.h"
#include <Imgui/imgui.h>
#include <Utility/Hash.h>

using namespace smug;
ToneMapProgram::ToneMapProgram() {

}

ToneMapProgram::~ToneMapProgram() {

}

void ToneMapProgram::Init(VkDevice& device, const glm::vec2& screenSize, FrameBufferManager& fbo, VkDescriptorPool& descPool, VkRenderPass& rp, DeviceAllocator& allocator) {
	VkPipelineVertexInputStateCreateInfo vertexInfo = {};
	vertexInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInfo.pNext = nullptr;
	m_Pipeline.SetDefaultVertexState(vertexInfo);
	m_Pipeline.LoadPipelineFromFile(device, "shader/ToneMap.json", rp);
	//pepare desc sets
	VkDescriptorSetAllocateInfo setAllocInfo;
	setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	setAllocInfo.pNext = nullptr;
	setAllocInfo.descriptorPool = descPool;
	setAllocInfo.descriptorSetCount = 1;
	setAllocInfo.pSetLayouts = m_Pipeline.GetDescriptorSetLayouts().data();

	for (uint32_t i = 0; i < BUFFER_COUNT; i++) {
		vkAllocateDescriptorSets(device, &setAllocInfo, &m_DescSet[i]);
	}

	VkSamplerCreateInfo sampInfo = {};
	sampInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampInfo.pNext = nullptr;
	sampInfo.addressModeU = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeV = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampInfo.addressModeW = VkSamplerAddressMode::VK_SAMPLER_ADDRESS_MODE_REPEAT;
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

	//each framebuffer texture + uniform buffer
	VkWriteDescriptorSet writeSet[1 + 2];
	VkDescriptorImageInfo imageInfo[1];
	VkDescriptorBufferInfo bufferInfo;

	//allocate memory for buffer
	m_Buffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(ToneMapUniformData));
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;
	bufferInfo.buffer = m_Buffer.buffer;

	for (uint32_t i = 0; i < 1; i++) {
		imageInfo[i].imageLayout = VkImageLayout::VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[i].sampler = m_Sampler;
		imageInfo[i].imageView = fbo.GetRenderTarget(HashString("HDR")).View;

		writeSet[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet[i].pNext = nullptr;
		writeSet[i].descriptorCount = 1;
		writeSet[i].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeSet[i].dstArrayElement = 0;
		writeSet[i].dstBinding = 0;
		writeSet[i].pImageInfo = &imageInfo[i];
		writeSet[i].dstSet = m_DescSet[i];
	}

	for (uint32_t i = 0; i < 2; i++) {
		writeSet[1 + i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet[1 + i].pNext = nullptr;
		writeSet[1 + i].descriptorCount = 1;
		writeSet[1 + i].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeSet[1 + i].dstArrayElement = 0;
		writeSet[1 + i].dstBinding = 1;
		writeSet[1 + i].dstSet = m_DescSet[i];
		writeSet[1 + i].pBufferInfo = &bufferInfo;
	}

	vkUpdateDescriptorSets(device, 3, writeSet, 0, nullptr);
}

void ToneMapProgram::DeInit(DeviceAllocator& allocator) {
	allocator.DeAllocateBuffer(m_Buffer);
}

void ToneMapProgram::Update(DeviceAllocator& allocator) {
	static ToneMapUniformData data = {1.0f, 1.0f / 1.6f};
	allocator.UpdateBuffer(m_Buffer, sizeof(ToneMapUniformData), &data);
}

void ToneMapProgram::Render(CommandBuffer& cmdBuffer, VkViewport viewport, uint32_t frameIndex) {
	vkCmdBindPipeline(cmdBuffer.CmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipeline());
	vkCmdBindDescriptorSets(cmdBuffer.CmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescSet[0], 0, nullptr);
	vkCmdSetViewport(cmdBuffer.CmdBuffer(), 0, 1, &viewport);
	//no need for a vertex buffer
	vkCmdDraw(cmdBuffer.CmdBuffer(), 3, 1, 0, 0);
}