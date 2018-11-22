#include "RenderCommands.h"

namespace smug {
	namespace RenderCommands {

		void RenderCommand(const RenderCmd& cmd, vk::CommandBuffer cmdBuffer, RenderQueue& rq, ResourceHandler& resources, const vk::DescriptorSet& perFrameSet, const vk::DescriptorSet& iblSet)
		{
			if (rq.GetModels().size() == 0) {
				return;
			}
			vk::DescriptorSet sets[] = { perFrameSet, iblSet, rq.GetDescriptorSet() };
			const PipelineState& ps = resources.GetPipelineState(cmd.Pipeline);
			cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, ps.GetPipelineLayout(), 0, _countof(sets), sets, 0, nullptr);
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, ps.GetPipeline());
			auto& models = rq.GetModels();
			for(auto& m : models){
				//bind index/vert buffers
				const Model& model = resources.GetModel(m.first);
				cmdBuffer.bindIndexBuffer(model.IndexBuffer.buffer, 0, vk::IndexType::eUint32);
				const vk::Buffer vbos[] = { model.VertexBuffers[0].buffer, model.VertexBuffers[1].buffer, model.VertexBuffers[2].buffer, model.VertexBuffers[3].buffer };
				const vk::DeviceSize offsets[] = { 0,0,0,0 };
				static_assert(_countof(vbos) == _countof(offsets), "missmatched array count");
				cmdBuffer.bindVertexBuffers(0, _countof(vbos), vbos, offsets);
				cmdBuffer.pushConstants(ps.GetPipelineLayout(), vk::ShaderStageFlagBits::eAll, 0, sizeof(unsigned), &m.second.Offset);
				for (uint32_t meshIndex = 0; meshIndex < model.MeshCount; meshIndex++) {
					const Mesh& mesh = model.Meshes[meshIndex];
					//bind material desc set
					const vk::DescriptorSet materialSet[] = { mesh.Material.DescSet };
					cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, ps.GetPipelineLayout(), _countof(sets), _countof(materialSet), materialSet, 0, nullptr);
					cmdBuffer.drawIndexed(mesh.IndexCount, m.second.Count, mesh.IndexOffset, 0, 0);
				}
			}
		}

	}
}