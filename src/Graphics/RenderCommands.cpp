#include "RenderCommands.h"

namespace smug {
	namespace RenderCommands {

		void RenderCommand(const RenderCmd& cmd, VkCommandBuffer cmdBuffer, RenderQueue& rq, ResourceHandler& resources, const VkDescriptorSet& perFrameSet, const VkDescriptorSet& iblSet)
		{
			//if (rq.GetModels().size() == 0) {
			//	return;
			//}
			//VkDescriptorSet sets[] = { perFrameSet, iblSet, rq.GetDescriptorSet() };
			//const PipelineState& ps = resources.GetPipelineState(cmd.Pipeline);
			//cmdBuffer.bindDescriptorSets(VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, ps.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
			//cmdBuffer.bindPipeline(VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, ps.GetPipeline());
			//auto& models = rq.GetModels();
			//for(auto& m : models){
			//	//bind index/vert buffers
			//	const Model& model = resources.GetModel(m.first);
			//	cmdBuffer.bindIndexBuffer(model.IndexBuffer.buffer, 0, VkIndexType::eUint32);
			//	const VkBuffer vbos[] = { model.VertexBuffers[0].buffer, model.VertexBuffers[1].buffer, model.VertexBuffers[2].buffer, model.VertexBuffers[3].buffer };
			//	const VkDeviceSize offsets[] = { 0,0,0,0 };
			//	static_assert(_countof(vbos) == _countof(offsets), "missmatched array count");
			//	cmdBuffer.bindVertexBuffers(0, _countof(vbos), vbos, offsets);
			//	cmdBuffer.pushConstants(ps.GetPipelineLayout(), VkShaderStageFlagBits::eAll, 0, sizeof(unsigned), &m.second.Offset);
			//	for (uint32_t meshIndex = 0; meshIndex < model.MeshCount; meshIndex++) {
			//		const Mesh& mesh = model.Meshes[meshIndex];
			//		const VkDescriptorSet materialSet[] = { mesh.Material.DescSet };
			//		cmdBuffer.bindDescriptorSets(VkPipelineBindPoint::eGraphics, ps.GetPipelineLayout(), _countof(sets), _countof(materialSet), materialSet, 0, nullptr);
			//		cmdBuffer.drawIndexed(mesh.IndexCount, m.second.Count, mesh.IndexOffset, 0, 0);
			//	}
			//}
		}

		void DisptachCommand(const DispatchCmd& cmd, VkCommandBuffer cmdBuffer, ResourceHandler& resources) {
			//const PipelineState& ps = resources.GetPipelineState(cmd.Shader);
			////TODO: bind pipeline resources
			////TODO: expose multiple queues
			//cmdBuffer.bindPipeline(VkPipelineBindPoint::eCompute, ps.GetPipeline());
			//cmdBuffer.dispatch(cmd.WorkGroupCountX, cmd.WorkGroupCountY, cmd.WorkGroupCountZ);
		}

		void CopyCommands(const CopyCmd& cmd, VkCommandBuffer cmdBuffer, ResourceHandler& resources) {
			//RESOURCE_TYPE srcType = GetType(cmd.Src);
			//RESOURCE_TYPE dstType = GetType(cmd.Dst);
			//if (srcType == RT_BUFFER && dstType == RT_BUFFER) {
			//	VkBuffer srcBuffer = resources.GetBuffer(cmd.Src).buffer;
			//	VkBuffer dstBuffer = resources.GetBuffer(cmd.Dst).buffer;
			//	VkBufferCopy copy;
			//	copy.dstOffset = cmd.OffsetDst;
			//	copy.srcOffset = cmd.OffsetSrc;
			//	copy.size = cmd.Size;
			//	cmdBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copy);
			//}
			//else {
			//	assert(false); //Unimplemented copy config
			//}
			
		}

		void FenceCommand(const FenceCmd& cmd, VkDevice device) {
		}

	}
}