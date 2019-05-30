#pragma once
#include <stdint.h>
#include "../AssetExport.h"
#include <AssetLoader/Resources.h>
#include <EASTL/vector.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>
namespace smug {
	//angelscript interface for building rendercommands
	namespace if_shader {

		struct VertexInput {
			VkFormat Format;
			uint16_t Binding;
			uint16_t Stream;
		};

		struct ScriptPipelineState {
			eastl::vector<ShaderByteCode> shaders;
			VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
			VkPipelineRasterizationStateCreateInfo RasterInfo;
			VkPipelineColorBlendStateCreateInfo BlendInfo;
			eastl::vector<VkPipelineColorBlendAttachmentState> AttachmentInfos;
			VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
			eastl::vector<VertexInput> VertexInputLayout;
			uint32_t RenderPass;
		};

		ScriptPipelineState* GetPSO(uint32_t hash);
		ASSET_DLL void InitInterface();
	}
}