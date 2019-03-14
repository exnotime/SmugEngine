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
	std::vector<VkFormat> formats;
	std::vector<VkImageUsageFlags> usages;
	formats.push_back(VkFormat::VK_FORMAT_D32_SFLOAT);
	usages.push_back(VkImageUsageFlagBits::VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VkImageUsageFlagBits::VK_IMAGE_USAGE_SAMPLED_BIT);
	m_FrameBuffer.Init(vc.Device, vc.PhysicalDevice, glm::vec2(SHADOWMAP_SIZE * 2), formats, usages);

	m_State.LoadPipelineFromFile(vc.Device, "shader/shadowmap.json", m_FrameBuffer.GetRenderPass());

	//allocate uniform buffer for light view matrixes
	m_UniformBuffer = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4) * 4, nullptr);
	//allocate descriptor set
	VkDescriptorSetAllocateInfo descAllocInfo = {};
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = nullptr;
	descAllocInfo.descriptorPool = vc.DescriptorPool;
	std::vector<VkDescriptorSetLayout>& descLayouts = m_State.GetDescriptorSetLayouts();
	descAllocInfo.pSetLayouts = descLayouts.data();
	descAllocInfo.descriptorSetCount = (uint32_t)descLayouts.size();
	vkAllocateDescriptorSets(vc.Device, &descAllocInfo, &m_DescSet);
	//fill in descriptor set
	VkWriteDescriptorSet writeInfo = {};
	writeInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeInfo.pNext = nullptr;
	writeInfo.descriptorCount = 1;
	writeInfo.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeInfo.dstBinding = 0;
	writeInfo.dstArrayElement = 0;
	writeInfo.dstSet = m_DescSet;
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = m_UniformBuffer.buffer;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;
	writeInfo.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(vc.Device, 1, &writeInfo, 0, nullptr);

	m_BoxHandle = g_AssetLoader.LoadAsset("assets/models/cube/cube.obj");

}

void ShadowMapProgram::DeInit(DeviceAllocator& allocator) {
	allocator.DeAllocateBuffer(m_UniformBuffer);
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

void ShadowMapProgram::Render(uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources, VulkanProfiler& profiler) {

	if (rq.GetModels().size() == 0) {
		return;
	}
	cmdBuffer.Begin(m_FrameBuffer.GetFrameBuffer(), m_FrameBuffer.GetRenderPass());
	profiler.Stamp(cmdBuffer.CmdBuffer(), "ShadowMap");
	static bool first = true;
	if(first) {
		std::vector<VkImageLayout> newLayouts;
		newLayouts.push_back(VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		m_FrameBuffer.ChangeLayout(cmdBuffer, newLayouts);
		cmdBuffer.PushPipelineBarrier();
		first = false;
	}

	VkRenderPassBeginInfo rpBeginInfo = {};
	rpBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rpBeginInfo.pNext = nullptr;
	rpBeginInfo.clearValueCount = 1;
	VkClearValue cv;
	cv.depthStencil.depth = 1.0f;
	cv.depthStencil.stencil = 0x0;
	VkClearValue cv2;
	VkClearValue clears[] = { cv, cv2 };
	rpBeginInfo.pClearValues = clears;
	rpBeginInfo.renderPass = m_FrameBuffer.GetRenderPass();
	rpBeginInfo.framebuffer = m_FrameBuffer.GetFrameBuffer();
	rpBeginInfo.renderArea.extent.width = (uint32_t)(SHADOWMAP_SIZE * 2);
	rpBeginInfo.renderArea.extent.height = (uint32_t)(SHADOWMAP_SIZE * 2);

	vkCmdBeginRenderPass(cmdBuffer.CmdBuffer(), &rpBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(cmdBuffer.CmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_State.GetPipeline());
	VkViewport viewports[CASCADE_COUNT] = {
		{ 0,				0,					SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ SHADOWMAP_SIZE,	0,					SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ 0,				SHADOWMAP_SIZE,		SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
		{ SHADOWMAP_SIZE,	SHADOWMAP_SIZE,		SHADOWMAP_SIZE,	SHADOWMAP_SIZE, 0.0f, 1.0f },
	};
	vkCmdSetViewport(cmdBuffer.CmdBuffer(), 0, CASCADE_COUNT, viewports);

	VkRect2D scissors[CASCADE_COUNT] = {
		{ {0,				0}, 			{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {SHADOWMAP_SIZE,	0}, 			{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {0,				SHADOWMAP_SIZE},{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
		{ {SHADOWMAP_SIZE,	SHADOWMAP_SIZE},{SHADOWMAP_SIZE,	SHADOWMAP_SIZE}},
	};
	vkCmdSetScissor(cmdBuffer.CmdBuffer(), 0, CASCADE_COUNT, scissors);

	VkDescriptorSet sets[] = { m_DescSet, rq.GetDescriptorSet() };
	vkCmdBindDescriptorSets(cmdBuffer.CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_State.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
	auto& models = rq.GetModels();
	for (auto& m : models) {
		const Model& model = resources.GetModel(m.first);
		const VkBuffer vertexBuffers[1] = { model.VertexBuffers[0].buffer };
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(cmdBuffer.CmdBuffer(), 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(cmdBuffer.CmdBuffer(), model.IndexBuffer.buffer, 0, VkIndexType::VK_INDEX_TYPE_UINT32);
		vkCmdPushConstants(cmdBuffer.CmdBuffer(), m_State.GetPipelineLayout(), VkShaderStageFlagBits::VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t), &m.second.Offset);
		for (uint32_t meshIt = 0; meshIt < model.MeshCount; ++meshIt) {
			Mesh& mesh = model.Meshes[meshIt];
			vkCmdDrawIndexed(cmdBuffer.CmdBuffer(), mesh.IndexCount, m.second.Count, mesh.IndexOffset, 0, 0);
		}
	}
	vkCmdEndRenderPass(cmdBuffer.CmdBuffer());
}