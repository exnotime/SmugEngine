#include "SkyBox.h"
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>
#include <glm/gtx/transform.hpp>
using namespace smug;

SkyBox::SkyBox() {

}
SkyBox::~SkyBox() {
}

void SkyBox::Init(const VkDevice& device, const VkPhysicalDevice& physDev, const std::string& filename,const VkViewport& vp, const VkRenderPass& rp, DeviceAllocator& allocator) {
	//load pipestate
	m_Pipeline.LoadPipelineFromFile(device, "shader/Skybox.json", rp);
	//allocate memory
	m_Texture.Init(filename,&allocator, device);
	//generate model
	par_shapes_mesh* mesh = par_shapes_create_cube();
	par_shapes_scale(mesh, 2, 2, 2);
	par_shapes_translate(mesh, -1, -1, -1);
	std::vector<glm::vec3> vertices;
	for (int i = 0; i < mesh->ntriangles * 3; i += 3) {
		for (int k = 0; k < 3; k++) {
			glm::vec3 p = glm::vec3(mesh->points[mesh->triangles[i + k] * 3], mesh->points[mesh->triangles[i + k] * 3 + 1], mesh->points[mesh->triangles[i + k] * 3 + 2]);
			vertices.push_back(p);
		}
	}

	m_VBO = allocator.AllocateBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.size() * sizeof(glm::vec3), vertices.data());
	m_UBO = allocator.AllocateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(glm::mat4) * 2, nullptr);

	par_shapes_free_mesh(mesh);
	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.pNext = nullptr;
	poolInfo.maxSets = 1;
	VkDescriptorPoolSize poolSizes[2];
	poolSizes[0].descriptorCount = 1;
	poolSizes[0].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 1;
	poolSizes[1].type = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;

	vkCreateDescriptorPool(device, &poolInfo,nullptr, &m_DescPool);

	VkDescriptorSetAllocateInfo descAllocInfo = {};
	descAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descAllocInfo.pNext = nullptr;
	descAllocInfo.descriptorPool = m_DescPool;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = m_Pipeline.GetDescriptorSetLayouts().data();
	vkAllocateDescriptorSets(device, &descAllocInfo, &m_DescSet);

	VkWriteDescriptorSet writeDescs[2];
	writeDescs[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescs[0].pNext = nullptr;
	writeDescs[0].descriptorCount = 1;
	writeDescs[0].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescs[0].dstArrayElement = 0;
	writeDescs[0].dstBinding = 0;
	writeDescs[0].dstSet = m_DescSet;
	VkDescriptorBufferInfo descBufferInfo = {};
	descBufferInfo.buffer = m_UBO.buffer;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	writeDescs[0].pBufferInfo = &descBufferInfo;
	writeDescs[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescs[1].pNext = nullptr;
	writeDescs[1].descriptorCount = 1;
	writeDescs[1].descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescs[1].dstArrayElement = 0;
	writeDescs[1].dstBinding = 1;
	writeDescs[1].dstSet = m_DescSet;
	VkDescriptorImageInfo imgInfo = m_Texture.GetDescriptorInfo();
	writeDescs[1].pImageInfo = &imgInfo;
	vkUpdateDescriptorSets(device, _countof(writeDescs), &writeDescs[0], 0, nullptr);
}

void SkyBox::PrepareUniformBuffer(DeviceAllocator& allocator, const glm::mat4& viewProj, const glm::mat4& world) {
	struct perFrameBuffer {
		glm::mat4 vp;
		glm::mat4 w;
	} pfb;
	pfb.vp = viewProj;
	pfb.w = world;
	allocator.UpdateBuffer(m_UBO, sizeof(perFrameBuffer), &pfb);
}

void SkyBox::Render(CommandBuffer* cmdBuffer) {
	vkCmdBindPipeline(cmdBuffer->CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipeline());
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(cmdBuffer->CmdBuffer(), 0, 1, &m_VBO.buffer, &offset);
	vkCmdBindDescriptorSets(cmdBuffer->CmdBuffer(), VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescSet, 0, nullptr);
	vkCmdDraw(cmdBuffer->CmdBuffer(), 36, 1, 0, 0);
}

void SkyBox::DeInit(DeviceAllocator& allocator) {
	allocator.DeAllocateBuffer(m_UBO);
	allocator.DeAllocateBuffer(m_VBO);
	allocator.DeAllocateImage(m_Texture.GetImageHandle());
}