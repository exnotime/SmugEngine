#include "GeometryProgram.h"
#include "DescriptorSetManager.h"

using namespace smug;

void GeometryProgram::Init(VulkanContext& vc, DeviceAllocator& allocator) {
	//m_PipelineState.LoadPipelineFromFile(vc.Device, "shader/filled.json", m_FrameBuffer.GetRenderPass());
}

void GeometryProgram::Update(DeviceAllocator& allocator, const RenderQueue& rq) {

}

void GeometryProgram::Render(CommandBuffer& cmdBuffer, const RenderQueue& rq, const ResourceHandler& resources) {
	if (rq.GetModels().size() == 0) {
		return;
	}
	//set up renderpass
	//bind pipeline state
	//render models

	//bind descriptor set
	//std::vector<VkDescriptorSet> sets;
	//sets.push_back(GetDescriptorSet("PerFrame"));
	//sets.push_back(GetDescriptorSet("IBL"));
	//sets.push_back(rq.GetDescriptorSet());
	//cmdBuffer.bindDescriptorSets(VkPipelineBindPoint::eGraphics, m_PipelineState.GetPipelineLayout(), 0, (uint32_t)sets.size(), sets.data(), 0, nullptr);

	//auto& models = rq.GetModels();
	//for (auto& m : models) {
	//	uint32_t instanceCount = m.second.Count;

	//	const Model& model = resources.GetModel(m.first);
	//	const VkBuffer vertexBuffers[4] = { model.VertexBuffers[0].buffer, model.VertexBuffers[1].buffer,
	//	                                      model.VertexBuffers[2].buffer, model.VertexBuffers[3].buffer
	//	                                    };

	//	const VkDeviceSize offsets[4] = { 0,0,0,0 };
	//	cmdBuffer.bindVertexBuffers(0, 4, vertexBuffers, offsets);
	//	cmdBuffer.bindIndexBuffer(model.IndexBuffer.buffer, 0, VkIndexType::eUint16);
	//	cmdBuffer.pushConstants(m_PipelineState.GetPipelineLayout(), VkShaderStageFlagBits::eAll, 0, sizeof(unsigned), &m.second.Offset);
	//	VkDescriptorSet mat;
	//	for (uint32_t meshIt = 0; meshIt < model.MeshCount; ++meshIt) {
	//		Mesh& mesh = model.Meshes[meshIt];
	//		if (mat != mesh.Material) {
	//			mat = mesh.Material;
	//			cmdBuffer.bindDescriptorSets(VkPipelineBindPoint::eGraphics, m_PipelineState.GetPipelineLayout(), 3, 1, &mesh.Material, 0, nullptr);
	//		}
	//		cmdBuffer.drawIndexed(mesh.IndexCount, instanceCount, mesh.IndexOffset, 0, 0);
	//	}
	//}
	//end renderpass

}
