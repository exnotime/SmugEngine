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
	//contruct the light view boxes
	//view plane corners
	glm::vec3 corners[] = {
		{-1,1,0},
		{1,1,0},
		{-1,-1,0},
		{1,-1,0}
	};
	const CameraData cd = rq.GetCameras()[0];
	glm::mat4 invViewProj = glm::inverse(cd.ProjView);
	glm::vec4 camViewCorners[5][4];
	for (int i = 0; i < 5; i++) {
		camViewCorners[i][0] = invViewProj * glm::vec4(corners[0], 1);
		camViewCorners[i][1] = invViewProj * glm::vec4(corners[1], 1);
		camViewCorners[i][2] = invViewProj * glm::vec4(corners[2], 1);
		camViewCorners[i][3] = invViewProj * glm::vec4(corners[3], 1);
		camViewCorners[i][0] /= camViewCorners[i][0].w;
		camViewCorners[i][1] /= camViewCorners[i][1].w;
		camViewCorners[i][2] /= camViewCorners[i][2].w;
		camViewCorners[i][3] /= camViewCorners[i][3].w;
		corners[0].z = i * 0.25f + 0.25f;
		corners[1].z = i * 0.25f + 0.25f;
		corners[2].z = i * 0.25f + 0.25f;
		corners[3].z = i * 0.25f + 0.25f;
	}
	//global light view matrix
	glm::vec3 forward = glm::normalize(light.Direction);
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));
	glm::mat4 lightView = glm::mat4(1);//identity matrix
	lightView[0] = glm::vec4(right, 0);
	lightView[1] = glm::vec4(up, 0);
	lightView[2] = glm::vec4(forward, 0);
	//light view aligned corners
	glm::vec4 lightCorners[4][8];
	for (int i = 0; i < 4; i++) {
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

	//transform bounds to light space
	glm::vec3 min[4], max[4];
	for (int i = 0; i < 4; i++) {
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
	int i = 0;
}