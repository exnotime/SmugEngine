#include "ShadowMapProgram.h"
#include <glm/gtx/transform.hpp>
#include <AssetLoader/AssetLoader.h>
#include <Core/Input.h>
using namespace smug;

ShadowMapProgram::ShadowMapProgram() {

}
ShadowMapProgram::~ShadowMapProgram() {

}

#define SHADOWMAP_SIZE 1024
#define CASCADE_COUNT 4

void ShadowMapProgram::Init(VulkanContext& vc, DeviceAllocator& allocator) {
	m_Device = &vc.Device;
	std::vector<vk::Format> formats;
	std::vector<vk::ImageUsageFlags> usages;
	formats.push_back(vk::Format::eD32Sfloat);
	usages.push_back(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	m_FrameBuffer.Init(vc.Device, vc.PhysicalDevice, glm::vec2(SHADOWMAP_SIZE * 2), formats, usages, 1);

	m_State.LoadPipelineFromFile(vc.Device, "shader/shadowmap.json", m_FrameBuffer.GetRenderPass());

	//allocate uniform buffer for light view matrixes
	m_UniformBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4) * 4, nullptr);
	//allocate descriptor set
	vk::DescriptorSetAllocateInfo descAllocInfo;
	descAllocInfo.descriptorPool = vc.DescriptorPool;
	std::vector<vk::DescriptorSetLayout>& descLayouts = m_State.GetDescriptorSetLayouts();
	descAllocInfo.pSetLayouts = descLayouts.data();
	descAllocInfo.descriptorSetCount = descLayouts.size();

	m_DescSet = vc.Device.allocateDescriptorSets(descAllocInfo)[0];
	//fill in descriptor set
	vk::WriteDescriptorSet writeInfo;
	writeInfo.descriptorCount = 1;
	writeInfo.descriptorType = vk::DescriptorType::eUniformBuffer;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = 0;
	writeInfo.dstSet = m_DescSet;
	vk::DescriptorBufferInfo bufferInfo;
	bufferInfo.buffer = m_UniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;
	writeInfo.pBufferInfo = &bufferInfo;
	vc.Device.updateDescriptorSets(1, &writeInfo, 0, nullptr);


	m_BoxHandle = g_AssetLoader.LoadAsset("assets/models/cube/cube.obj");

}

void ShadowMapProgram::Update(DeviceAllocator& allocator, RenderQueue& rq) {
	if (g_Input.IsKeyPushed(GLFW_KEY_O)) {
		m_DebugMode = !m_DebugMode;
		m_DebugCamData = rq.GetCameras()[0];
	}

	//contruct the light view boxes
	DirLight dl;
	dl.Direction = glm::vec3(0.1f, -1.0f, -0.5f);
	dl.Color = glm::vec4(1);
	const CameraData& cd = m_DebugMode ? m_DebugCamData : rq.GetCameras()[0];

	float cascadeSplits[CASCADE_COUNT];

	float nearClip = cd.Near;
	float farClip = cd.Far;
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// Calculate split depths based on view camera furstum
	// Based on method presentd in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	const float cascadeSplitLambda = 0.95f;
	for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
		float p = (i + 1) / static_cast<float>(CASCADE_COUNT);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = cascadeSplitLambda * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	// Calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
		float splitDist = cascadeSplits[i];

		glm::vec3 frustumCorners[8] = {
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f),
		};

		// Project frustum corners into world space
		glm::mat4 invCam = glm::inverse(cd.ProjView);
		for (uint32_t i = 0; i < 8; i++) {
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[i], 1.0f);
			frustumCorners[i] = invCorner / invCorner.w;
		}

		for (uint32_t i = 0; i < 4; i++) {
			glm::vec3 dist = frustumCorners[i + 4] - frustumCorners[i];
			frustumCorners[i + 4] = frustumCorners[i] + (dist * splitDist);
			frustumCorners[i] = frustumCorners[i] + (dist * lastSplitDist);
		}

		// Get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t i = 0; i < 8; i++) {
			frustumCenter += frustumCorners[i];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t i = 0; i < 8; i++) {
			float distance = glm::length(frustumCorners[i] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		glm::vec3 lightDir = glm::normalize(dl.Direction);
		glm::mat4 lightViewMatrix = glm::lookAt(frustumCenter - lightDir * radius * 8.0f, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightOrthoMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, 0.0f, 1000.0f);

		// Store split distance and matrix in cascade
		m_ShadowSplits[i] = (cd.Near + splitDist * clipRange) * -1.0f;
		m_LightViewProjs[i] = lightOrthoMatrix * lightViewMatrix;
		m_ShadowSplits[i] = cascadeSplits[i];
		lastSplitDist = cascadeSplits[i];
	}

	allocator.UpdateBuffer(m_UniformBuffer, sizeof(glm::mat4) * CASCADE_COUNT, m_LightViewProjs);
}

void ShadowMapProgram::Render(uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources) {

	if (rq.GetModels().size() == 0) {
		return;
	}
	cmdBuffer.Begin(m_FrameBuffer.GetFrameBuffer(0), m_FrameBuffer.GetRenderPass());
	static bool first = true;
	if(first) {
		std::vector<vk::ImageLayout> newLayouts;
		newLayouts.push_back(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		m_FrameBuffer.ChangeLayout(cmdBuffer, newLayouts, 0);
		cmdBuffer.PushPipelineBarrier();
		first = false;
	}

	vk::RenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.clearValueCount = 1;
	vk::ClearValue cv;
	cv.depthStencil.depth = 1.0f;
	cv.depthStencil.stencil = 0x0;
	vk::ClearValue cv2;
	vk::ClearValue clears[] = { cv, cv2 };
	rpBeginInfo.pClearValues = clears;
	rpBeginInfo.renderPass = m_FrameBuffer.GetRenderPass();
	rpBeginInfo.framebuffer = m_FrameBuffer.GetFrameBuffer(0);
	rpBeginInfo.renderArea.extent.width = (uint32_t)(SHADOWMAP_SIZE * 2);
	rpBeginInfo.renderArea.extent.height = (uint32_t)(SHADOWMAP_SIZE * 2);
	cmdBuffer.beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_State.GetPipeline());
	vk::Viewport viewports[CASCADE_COUNT] = {
		{ 0,				0,					SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ SHADOWMAP_SIZE,	0,					SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ 0,				SHADOWMAP_SIZE,		SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ SHADOWMAP_SIZE,	SHADOWMAP_SIZE,		SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
	};
	cmdBuffer.setViewport(0, CASCADE_COUNT, viewports);

	vk::Rect2D scissors[CASCADE_COUNT] = {
		{ {0,				0}, 			{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {SHADOWMAP_SIZE,	0}, 			{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {0,				SHADOWMAP_SIZE},{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {SHADOWMAP_SIZE,	SHADOWMAP_SIZE},{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
	};
	cmdBuffer.setScissor(0, CASCADE_COUNT, scissors);


	vk::DescriptorSet sets[] = { m_DescSet, rq.GetDescriptorSet() };
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_State.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
	auto& models = rq.GetModels();
	for (auto& m : models) {
		const Model& model = resources.GetModel(m.first);
		const vk::Buffer vertexBuffers[1] = { model.VertexBuffers[0].buffer };
		const vk::DeviceSize offsets[1] = { 0 };
		cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
		cmdBuffer.bindIndexBuffer(model.IndexBuffer.buffer, 0, vk::IndexType::eUint16);
		cmdBuffer.pushConstants(m_State.GetPipelineLayout(), vk::ShaderStageFlagBits::eAll, 0, sizeof(uint32_t), &m.second.Offset);
		for (uint32_t meshIt = 0; meshIt < model.MeshCount; ++meshIt) {
			Mesh& mesh = model.Meshes[meshIt];
			cmdBuffer.drawIndexed(mesh.IndexCount, m.second.Count, mesh.IndexOffset, 0, 0);
		}
	}
	cmdBuffer.endRenderPass();
}