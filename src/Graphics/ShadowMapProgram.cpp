#include "ShadowMapProgram.h"
using namespace smug;

ShadowMapProgram::ShadowMapProgram() {

}
ShadowMapProgram::~ShadowMapProgram() {

}

#define SHADOWMAP_SIZE 1024

void ShadowMapProgram::Init(vk::Device device, vk::PhysicalDevice gpu, vk::Format format) {
	Vector<vk::Format> formats;
	formats.push_back(format);
	Vector<vk::ImageUsageFlags> usages;
	usages.push_back(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	m_FrameBuffer.Init(device, gpu, glm::vec2(SHADOWMAP_SIZE), formats, usages);

	m_State.LoadPipelineFromFile(device, "shader/shadowmap.json", m_FrameBuffer.GetRenderPass());
}

void ShadowMapProgram::Render(const RenderQueue& rq, const DirLight& light) {

}