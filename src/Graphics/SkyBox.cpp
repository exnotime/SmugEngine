#include "SkyBox.h"
#define PAR_SHAPES_IMPLEMENTATION
#include <par_shapes.h>
#include <glm/gtx/transform.hpp>
using namespace smug;

SkyBox::SkyBox() {

}
SkyBox::~SkyBox() {
}

void SkyBox::Init(const vk::Device& device, const vk::PhysicalDevice& physDev, const std::string& filename,const vk::Viewport& vp, const vk::RenderPass& rp, DeviceAllocator& allocator) {
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
	vk::DescriptorPoolCreateInfo poolInfo;
	poolInfo.maxSets = 1;
	vk::DescriptorPoolSize poolSizes[2];
	poolSizes[0].descriptorCount = 1;
	poolSizes[0].type = vk::DescriptorType::eUniformBuffer;
	poolSizes[1].descriptorCount = 1;
	poolSizes[1].type = vk::DescriptorType::eCombinedImageSampler;
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	m_DescPool = device.createDescriptorPool(poolInfo);

	vk::DescriptorSetAllocateInfo descAllocInfo;
	descAllocInfo.descriptorPool = m_DescPool;
	descAllocInfo.descriptorSetCount = 1;
	descAllocInfo.pSetLayouts = m_Pipeline.GetDescriptorSetLayouts().data();
	m_DescSet = device.allocateDescriptorSets(descAllocInfo)[0];

	std::array<vk::WriteDescriptorSet, 2> writeDescs;
	writeDescs[0].descriptorCount = 1;
	writeDescs[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	writeDescs[0].dstArrayElement = 0;
	writeDescs[0].dstBinding = 0;
	writeDescs[0].dstSet = m_DescSet;
	vk::DescriptorBufferInfo descBufferInfo;
	descBufferInfo.buffer = m_UBO.buffer;
	descBufferInfo.offset = 0;
	descBufferInfo.range = VK_WHOLE_SIZE;
	writeDescs[0].pBufferInfo = &descBufferInfo;
	writeDescs[1].descriptorCount = 1;
	writeDescs[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	writeDescs[1].dstArrayElement = 0;
	writeDescs[1].dstBinding = 1;
	writeDescs[1].dstSet = m_DescSet;
	vk::DescriptorImageInfo imgInfo = m_Texture.GetDescriptorInfo();
	writeDescs[1].pImageInfo = &imgInfo;
	device.updateDescriptorSets(writeDescs, nullptr);
}

void SkyBox::PrepareUniformBuffer(CommandBuffer cmdBuffer, DeviceAllocator& allocator, const glm::mat4& viewProj, const glm::mat4& world) {
	struct perFrameBuffer {
		glm::mat4 vp;
		glm::mat4 w;
	} pfb;
	pfb.vp = viewProj;
	pfb.w = world;
	allocator.UpdateBuffer(m_UBO, sizeof(perFrameBuffer), &pfb);
	allocator.ScheduleTransfers(&cmdBuffer);
}

void SkyBox::Render(CommandBuffer cmdBuffer) {
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipeline());
	vk::DeviceSize offset = 0;
	vk::Buffer buf(m_VBO.buffer);//vulkan.hpp!!! why?!
	cmdBuffer.bindVertexBuffers(0, 1, &buf, &offset);
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescSet, 0, nullptr);
	cmdBuffer.draw(36, 1, 0, 0);
}