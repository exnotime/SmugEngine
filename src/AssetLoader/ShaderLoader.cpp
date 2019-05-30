#include "ShaderLoader.h"
#include <json.hpp>
#include "script/if_Shader.h"
#include <AngelScript/ScriptEngine.h>
#include <angelscript.h>
#include <fstream>
#include <Utility/Hash.h>
#include <Utility/Memory.h>
#include <spirv_cross/spirv_cross.hpp>
#include <EASTL/vector.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

using namespace smug;
ShaderLoader::ShaderLoader() {}
ShaderLoader::~ShaderLoader() {}


char* ReflectShaders(const eastl::vector<ShaderByteCode>& shaders, PipelineStateInfo& psInfoOut){
	eastl::vector<Descriptor> descs;
	psInfoOut.DescriptorCount = 0;
	psInfoOut.PushConstants.Offset = 0;
	psInfoOut.PushConstants.Size = 0;
	uint32_t shaderCount = shaders.size();
	for (uint32_t i = 0; i < shaderCount; ++i) {
		const ShaderByteCode& shader = shaders[i];
		//since the fuckers who wrote spirv-cross cant write a destructor that does not crash i am just gonna leak this shit until it gets fixed.
		spirv_cross::Compiler* compiler = new spirv_cross::Compiler((const uint32_t*)shader.ByteCode, (size_t)shader.ByteCodeSize / sizeof(uint32_t));
		spirv_cross::ShaderResources resources = compiler->get_shader_resources();
		Descriptor d;
		d.Stage = shader.Kind;
		for (auto& res : resources.sampled_images) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descs.push_back(d);
		}
		for (auto& res : resources.separate_images) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descs.push_back(d);
		}
		for (auto& res : resources.separate_samplers) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_SAMPLER;
			descs.push_back(d);
		}
		for (auto& res : resources.uniform_buffers) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descs.push_back(d);
		}
		for (auto& res : resources.storage_buffers) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			descs.push_back(d);
		}
		for (auto& res : resources.storage_images) {
			d.Set = compiler->get_decoration(res.id, spv::DecorationDescriptorSet);
			d.Bindpoint = compiler->get_decoration(res.id, spv::DecorationBinding);
			d.Count = compiler->get_type(res.type_id).array.size() > 0 ? compiler->get_type(res.type_id).array[0] : 1;
			d.Resource = HashString((compiler->get_name(res.id).c_str()));
			d.Type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			descs.push_back(d);
		}
		for (auto& res : resources.push_constant_buffers) {
			psInfoOut.PushConstants.Size = 0;
			psInfoOut.PushConstants.Offset = UINT32_MAX;
			auto ranges = compiler->get_active_buffer_ranges(res.id);
			for (auto& r : ranges) {
				psInfoOut.PushConstants.Size += (uint32_t)r.range;
				psInfoOut.PushConstants.Offset = glm::min(psInfoOut.PushConstants.Offset, (uint32_t)r.offset);
			}
		}
		if (shader.Kind == SHADER_KIND::VERTEX) {
			for (auto& v : resources.stage_inputs) {

			}
		}
		delete compiler;
	}
	if (shaderCount) {
		psInfoOut.ShaderCount = shaderCount;
		psInfoOut.Shaders = (ShaderByteCode*)malloc(shaderCount * sizeof(ShaderByteCode));
		memcpy(psInfoOut.Shaders, shaders.data(), shaderCount * sizeof(ShaderByteCode));
	}

	if (!descs.empty()) {
		psInfoOut.Descriptors = (Descriptor*)malloc(sizeof(Descriptor) * descs.size());
		memcpy(psInfoOut.Descriptors, descs.data(), sizeof(Descriptor) * descs.size());
		psInfoOut.DescriptorCount = (uint32_t)descs.size();
	}
	
	return nullptr;
}

LoadResult ShaderLoader::LoadAsset(const char* filename) {
	char* error = nullptr;

	AngelScript::asIScriptModule* shaderModule = g_ScriptEngine.CompileScriptToModule(filename);
	if_shader::ScriptPipelineState* pso = nullptr;
	if (shaderModule) {
		g_ScriptEngine.ExecuteModule(shaderModule, "uint LoadPSO()");
		pso = if_shader::GetPSO(g_ScriptEngine.GetContext()->GetReturnDWord());
	}
	PipelineStateInfo* psInfo = nullptr;
	if (!error) {
		psInfo = (PipelineStateInfo*)malloc(sizeof(PipelineStateInfo));
		error = ReflectShaders(pso->shaders, *psInfo);
		psInfo->AttachmentCount = pso->AttachmentInfos.size();
		psInfo->Attachments = (VkPipelineColorBlendAttachmentState*)malloc(pso->AttachmentInfos.size() * sizeof(VkPipelineColorBlendAttachmentState));
		memcpy(psInfo->Attachments, pso->AttachmentInfos.data(), pso->AttachmentInfos.size() * sizeof(VkPipelineColorBlendAttachmentState));
		psInfo->DepthStencilState = pso->DepthStencilInfo;
		psInfo->RasterState = pso->RasterInfo;
		psInfo->Topology = pso->InputAssemblyInfo.topology;
		psInfo->RenderPass = pso->RenderPass;
	}
	LoadResult res;
	if (error) {
		res.Error = error;
	} else {
		res.Hash = HashString(filename);
		res.Data = psInfo;
		res.Type = RT_SHADER;
	}
	return res;
}

void ShaderLoader::UnloadAsset(void* asset) {
	PipelineStateInfo* info = (PipelineStateInfo*)asset;
	for (uint32_t i = 0; i < info->ShaderCount; ++i) {
		free(info->Shaders[i].ByteCode);
	}
	free(info->Attachments);
	free(info->Descriptors);
	free(info->Shaders);
	free(info);
}

//psoinfo
//shader byte code array
//shader data
//descriptors
//attachments

void WriteShaderToFile(FileBuffer* buffer, ShaderByteCode* sbc, uint32_t& offset, uint32_t hash) {
	offset += sizeof(ShaderByteCode);
	ShaderByteCode shader = *sbc;
	shader.ByteCode = (void*)offset;
	offset += shader.ByteCodeSize;
	buffer->Write(sizeof(ShaderByteCode), &shader, hash);
	buffer->Write(sbc->ByteCodeSize, sbc->ByteCode, hash);
}

void ShaderLoader::SerializeAsset(FileBuffer* buffer, LoadResult* asset)  {
	PipelineStateInfo& pi = *(PipelineStateInfo*)asset->Data;
	
	PipelineStateInfo psInfo = pi;
	uint32_t offset = sizeof(PipelineStateInfo);
	psInfo.Shaders = (ShaderByteCode*)offset;
	
	buffer->Write(sizeof(PipelineStateInfo), &psInfo, asset->Hash);
	for (uint32_t i = 0; i < pi.ShaderCount; ++i) {
		WriteShaderToFile(buffer, &pi.Shaders[i], offset, asset->Hash);
	}

	psInfo.Descriptors = (Descriptor*)offset;
	offset += sizeof(Descriptor) * pi.DescriptorCount;
	buffer->Write(sizeof(Descriptor) * pi.DescriptorCount, pi.Descriptors, asset->Hash);

	psInfo.Attachments = (VkPipelineColorBlendAttachmentState*)offset;
	offset += sizeof(VkPipelineColorBlendAttachmentState) * pi.AttachmentCount;
	buffer->Write(sizeof(VkPipelineColorBlendAttachmentState) * pi.DescriptorCount, pi.Descriptors, asset->Hash);

	//ShaderInfo& si = pi.Shader;
	//ps state = psInfo + shaders + descriptors + rendertargets
	//serialize shaders
	//uint32_t offset = sizeof(PipelineStateInfo) + sizeof(uint32_t) + sizeof(ShaderByteCode) * si.ShaderCount;
	//eastl::vector<ShaderByteCode> byteCodes;
	//for (uint32_t i = 0; i < si.ShaderCount; ++i) {
	//	ShaderByteCode bc = si.Shaders[i];
	//	bc.ByteCode = (void*)offset;
	//	offset += si.Shaders[i].ByteCodeSize;
	//	bc.DependenciesHashes = (uint32_t*)offset;
	//	offset += si.Shaders[i].DependencyCount * sizeof(uint32_t);
	//	byteCodes.push_back(bc);
	//}
	//PipelineStateInfo psInfo = pi;
	//psInfo.Descriptors = (Descriptor*)offset;
	//offset += sizeof(Descriptor) * pi.DescriptorCount;
	//psInfo.RenderTargets = (uint32_t*)offset;

	//buffer->Write(sizeof(PipelineStateInfo), &psInfo, asset->Hash);
	//buffer->Write(sizeof(uint32_t), (void*)&si.ShaderCount, asset->Hash);
	//buffer->Write(sizeof(ShaderByteCode) * byteCodes.size(), (void*)byteCodes.data(), asset->Hash);

	//for (uint32_t i = 0; i < si.ShaderCount; ++i) {
	//	buffer->Write(si.Shaders[i].ByteCodeSize, (void*)si.Shaders[i].ByteCode, asset->Hash);
	//	buffer->Write(sizeof(uint32_t) * si.Shaders[i].DependencyCount, (void*)si.Shaders[i].DependenciesHashes, asset->Hash);
	//}
	////serialize descriptors
	//buffer->Write(sizeof(Descriptor) * pi.DescriptorCount, pi.Descriptors, asset->Hash);
	////serialize render targets
	//buffer->Write(sizeof(uint32_t) * pi.RenderTargetCount, pi.RenderTargets, asset->Hash);

}

void LoadShaderFromFile(void* buffer, uint32_t& offset, ShaderByteCode* dest) {
	ShaderByteCode* src = (ShaderByteCode*)PointerAdd(buffer, offset);
	*dest = *src;
	dest->ByteCode = malloc(src->ByteCodeSize);
	memcpy(dest->ByteCode, PointerAdd(buffer, (uint32_t)src->ByteCode), src->ByteCodeSize);
	offset += sizeof(ShaderByteCode) + src->ByteCodeSize;
}

DeSerializedResult ShaderLoader::DeSerializeAsset(void* assetBuffer) {
	PipelineStateInfo* source = (PipelineStateInfo*)assetBuffer;
	PipelineStateInfo* dest = new PipelineStateInfo();
	*dest = *source;
	dest->Shaders = (ShaderByteCode*)malloc(sizeof(ShaderByteCode) * source->ShaderCount);
	uint32_t offset = sizeof(PipelineStateInfo);
	for (int i = 0; i < dest->ShaderCount; ++i) {
		LoadShaderFromFile(assetBuffer, offset, &dest->Shaders[i]);
	}
	dest->Descriptors = (Descriptor*)malloc(sizeof(Descriptor) * dest->DescriptorCount);
	memcpy(dest->Descriptors, PointerAdd(assetBuffer, (uint32_t)source->Descriptors), sizeof(Descriptor) * dest->DescriptorCount);

	dest->Attachments = (VkPipelineColorBlendAttachmentState*)malloc(sizeof(VkPipelineColorBlendAttachmentState) * dest->AttachmentCount);
	memcpy(dest->Attachments, PointerAdd(assetBuffer, (uint32_t)source->Attachments), sizeof(VkPipelineColorBlendAttachmentState) * dest->AttachmentCount);
	//ShaderInfo& sourceShader = source->Shader;
	////deserialize shaders
	//ShaderByteCode* destByteCode = (ShaderByteCode*)malloc(sizeof(ShaderByteCode) * sourceShader.ShaderCount);
	//uint32_t offset = sizeof(PipelineStateInfo) + sizeof(uint32_t);
	//ShaderByteCode* sourceByteCode = (ShaderByteCode*)PointerAdd(assetBuffer, offset);

	//memcpy(destByteCode, sourceByteCode, sizeof(ShaderByteCode) * sourceShader.ShaderCount);

	//for (uint32_t i = 0; i < sourceShader.ShaderCount; ++i) {
	//	destByteCode[i].ByteCode = malloc(sourceByteCode[i].ByteCodeSize);
	//	memcpy(destByteCode[i].ByteCode, PointerAdd(assetBuffer, (uint32_t)sourceByteCode[i].ByteCode), sourceByteCode[i].ByteCodeSize);

	//	destByteCode[i].DependenciesHashes = (uint32_t*)malloc(sourceByteCode[i].DependencyCount * sizeof(uint32_t));
	//	memcpy(destByteCode[i].DependenciesHashes, PointerAdd(assetBuffer,(uint32_t)sourceByteCode[i].DependenciesHashes), sourceByteCode[i].DependencyCount * sizeof(uint32_t));
	//}
	//dest->Shader.ShaderCount = sourceShader.ShaderCount;
	//dest->Shader.Shaders = destByteCode;
	////deserialize descriptors
	//dest->Descriptors = (Descriptor*)malloc(sizeof(Descriptor) * source->DescriptorCount);
	//memcpy(dest->Descriptors, PointerAdd(assetBuffer, (uint32_t)source->Descriptors), sizeof(Descriptor) * source->DescriptorCount);
	////deserialize render targets
	//dest->RenderTargets = (uint32_t*)malloc(sizeof(uint32_t) * source->RenderTargetCount);
	//memcpy(dest->RenderTargets, PointerAdd(assetBuffer, (uint32_t)source->RenderTargets), sizeof(uint32_t) * source->RenderTargetCount);

	DeSerializedResult res;
	res.Data = dest;
	res.Type = RT_SHADER;
	return res;
}

bool ShaderLoader::IsExtensionSupported(const char* extension) {
	const char* extensions[] = { "shader" };
	for (auto& ext : extensions) {
		if (strcmp(ext, extension) == 0)
			return true;
	}
	return false;
}


bool ShaderLoader::IsTypeSupported(int type) {
	return type == RT_SHADER;
}