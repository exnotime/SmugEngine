#include "ShadowMapProgram.h"
#include <glm/gtx/transform.hpp>
#include <AssetLoader/AssetLoader.h>
#include <Core/Input.h>
using namespace smug;

ShadowMapProgram::ShadowMapProgram() {

}
ShadowMapProgram::~ShadowMapProgram() {

}

#define SHADOWMAP_SIZE 2048
#define CASCADE_COUNT 4

void ShadowMapProgram::Init(VulkanContext& vc, DeviceAllocator& allocator) {
	Vector<vk::Format> formats;
	Vector<vk::ImageUsageFlags> usages;
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
	//view plane corners
	glm::vec3 corners[] = {
		{ -1,1,0 },
		{ 1,1,0 },
		{ -1,-1,0 },
		{ 1,-1,0 }
	};
	const CameraData& cd = m_DebugMode ? m_DebugCamData : rq.GetCameras()[0];
	glm::mat4 invViewProj = glm::inverse(cd.ProjView);
	glm::vec4 camViewCorners[5][4];
	glm::vec4 frustumMin = glm::vec4(INFINITY), frustumMax = glm::vec4(-INFINITY);
	float delinearizedDepths[CASCADE_COUNT + 1];
	for (int i = 0; i < CASCADE_COUNT + 1; i++) {
		float d = cd.Near + cd.Far * i * 0.25;
		glm::vec4 p = (cd.ProjView * glm::vec4(cd.Position + cd.Forward * d, 1));
		delinearizedDepths[i] = p.z / p.w;
	}
	for (int i = 0; i < CASCADE_COUNT + 1; i++) {
		corners[0].z = delinearizedDepths[i];
		corners[1].z = delinearizedDepths[i];
		corners[2].z = delinearizedDepths[i];
		corners[3].z = delinearizedDepths[i];
		camViewCorners[i][0] = invViewProj * glm::vec4(corners[0], 1);
		camViewCorners[i][1] = invViewProj * glm::vec4(corners[1], 1);
		camViewCorners[i][2] = invViewProj * glm::vec4(corners[2], 1);
		camViewCorners[i][3] = invViewProj * glm::vec4(corners[3], 1);
		camViewCorners[i][0] /= camViewCorners[i][0].w;
		camViewCorners[i][1] /= camViewCorners[i][1].w;
		camViewCorners[i][2] /= camViewCorners[i][2].w;
		camViewCorners[i][3] /= camViewCorners[i][3].w;

		frustumMin = glm::min(camViewCorners[i][0], frustumMin); frustumMax = glm::max(camViewCorners[i][0], frustumMax);
		frustumMin = glm::min(camViewCorners[i][1], frustumMin); frustumMax = glm::max(camViewCorners[i][1], frustumMax);
		frustumMin = glm::min(camViewCorners[i][2], frustumMin); frustumMax = glm::max(camViewCorners[i][2], frustumMax);
		frustumMin = glm::min(camViewCorners[i][3], frustumMin); frustumMax = glm::max(camViewCorners[i][3], frustumMax);
	}

	//global light view matrix
	glm::vec4 center = invViewProj * glm::vec4(0, 0, delinearizedDepths[2],1);
	center /= center.w;
	glm::vec4 lightPos = center - glm::vec4(glm::normalize(dl.Direction),0) * glm::distance(frustumMin, frustumMax);
	glm::mat4 lightView = glm::lookAt(glm::vec3(lightPos), glm::vec3(lightPos) + dl.Direction, glm::vec3(0, 1, 0));
	//light view aligned corners
	glm::vec4 lightCorners[CASCADE_COUNT][8];
	for (int i = 0; i < CASCADE_COUNT; i++) {
		//all cascades start at the near plane
		lightCorners[i][0] = lightView * camViewCorners[0][0];
		lightCorners[i][1] = lightView * camViewCorners[0][1];
		lightCorners[i][2] = lightView * camViewCorners[0][2];
		lightCorners[i][3] = lightView * camViewCorners[0][3];
		//each cascade then extend further into the view
		lightCorners[i][4] = lightView * camViewCorners[i + 1][0];
		lightCorners[i][5] = lightView * camViewCorners[i + 1][1];
		lightCorners[i][6] = lightView * camViewCorners[i + 1][2];
		lightCorners[i][7] = lightView * camViewCorners[i + 1][3];
	}							  
	
	// calc min max bound for each cascade
	glm::vec3 min[CASCADE_COUNT], max[CASCADE_COUNT];
	for (int i = 0; i < CASCADE_COUNT; i++) {
		min[i] = glm::vec3(INFINITY); max[i] = glm::vec3(-INFINITY);
		min[i] = glm::min(glm::vec3(lightCorners[i][0]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][0]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][1]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][1]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][2]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][2]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][3]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][3]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][4]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][4]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][5]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][5]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][6]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][6]), max[i]);
		min[i] = glm::min(glm::vec3(lightCorners[i][7]), min[i]); max[i] = glm::max(glm::vec3(lightCorners[i][7]), max[i]);
	}
	for (int i = 0; i < CASCADE_COUNT; i++) {
		m_LightViewProjs[i] = glm::ortho(min[i].x, max[i].x, min[i].y, max[i].y, 0.1f, 1000.0f) * lightView;
	}

	allocator.UpdateBuffer(m_UniformBuffer, sizeof(glm::mat4) * CASCADE_COUNT, m_LightViewProjs);

	if (m_DebugMode) {
		rq.AddModel(m_BoxHandle, glm::translate(glm::vec3(lightPos)), glm::vec4(1, 0, 1, 0));
		for (uint32_t i = 0; i < CASCADE_COUNT; i++) {
			glm::vec3 pos = min[i] + max[i] * 0.5f;
			glm::mat4 transform = glm::translate(pos) * glm::scale(glm::vec3(30));
			static const glm::vec4 colors[] = { {1,0,0,1}, {0,1,0,1}, {0,0,1,1}, {1,1,1,1} };
			rq.AddModel(m_BoxHandle, transform, colors[i]);
		}
	}

}

void ShadowMapProgram::Render(uint32_t frameIndex, CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources) {

	if (rq.GetModels().size() == 0) {
		return;
	}
	cmdBuffer.Begin(m_FrameBuffer.GetFrameBuffer(0), m_FrameBuffer.GetRenderPass());
	static bool first = true;
	if(first){
		Vector<vk::ImageLayout> newLayouts;
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