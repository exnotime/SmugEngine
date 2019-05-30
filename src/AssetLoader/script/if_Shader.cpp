#pragma once
#include "if_Shader.h"
#include <assert.h>
#include <AngelScript/ScriptEngine.h>
#include <EASTL/unordered_map.h>
#include <Graphics/VkShader.h>
#include <Graphics/PipelineCommon.h>
#include <Utility/Hash.h>

namespace smug {
	//angelscript interface for building pipeline states and shaders
	namespace if_shader {

		static eastl::unordered_map<uint32_t, ScriptPipelineState> psos;

		uint32_t CreatePSO(eastl::string name) {
			uint32_t hashedName = HashString(name);
			if (psos.find(hashedName) == psos.end()) {
				psos[hashedName] = ScriptPipelineState();
				psos[hashedName].BlendInfo = GetDefaultColorBlendState();
				psos[hashedName].DepthStencilInfo = GetDefaultDepthStencilstate();
				psos[hashedName].RasterInfo = GetDefaultRasterState();
				psos[hashedName].InputAssemblyInfo = GetDefaultInputAssemblyState();
			}
			return hashedName;
		}

		ShaderByteCode LoadShaderByteCode(const eastl::string& filename, SHADER_KIND stage, const eastl::string& entryPoint, SHADER_LANGUAGE language) {
			//use the program glslangvalidator
			eastl::string command;
			command += "%VULKAN_SDK%/Bin/glslangValidator.exe -V ";
			switch (stage) {
			case smug::VERTEX:
				command += "-S vert -DVERTEX";
				break;
			case smug::FRAGMENT:
				command += "-S frag -DFRAGMENT";
				break;
			case smug::GEOMETRY:
				command += "-S geom -DGEOMETRY";
				break;
			case smug::EVALUATION:
				command += "-S tese -DEVALUATION";
				break;
			case smug::CONTROL:
				command += "-S tesc -DCONTROL";
				break;
			case smug::COMPUTE:
				command += "-S comp -DCOMPUTE";
				break;
			case smug::MESH:
				command += "-S mesh -DMESH";
				break;
			case smug::TASK:
				command += "-S task -DTASK";
				break;
			case smug::RAY_GEN:
				command += "-S rgen -DRAY_GEN";
				break;
			case smug::RAY_ANY_HIT:
				command += "-S rahit -DRAY_ANY_HIT";
				break;
			case smug::RAY_CLOSEST_HIT:
				command += "-S rchit -DRAY_CLOSEST_HIT";
				break;
			case smug::RAY_MISS:
				command += "-S rmiss -DRAY_MISS";
				break;
			case smug::RAY_INTERSECTION:
				command += "-S rint -DRAY_INTERSECTION";
				break;
			case smug::RAY_CALLABLE:
				command += "-S rcall -DRAY_CALLABLE";
				break;
			}
			command += " -e " + entryPoint;


			command += " -Ishader/";
			command += " -o ./temp.spv";
			command += " ./" + filename;

			int ret = system(command.c_str());
			ShaderByteCode sbc = {};
			FILE* fin = fopen("./temp.spv", "rb");
			if (fin) {

				fseek(fin, 0, SEEK_END);
				sbc.ByteCodeSize = ftell(fin);
				rewind(fin);
				sbc.ByteCode = (uint8_t*)malloc(sbc.ByteCodeSize);
				fread(sbc.ByteCode, sizeof(uint8_t), sbc.ByteCodeSize, fin);
				sbc.Kind = stage;
				sbc.SrcLanguage = language;
				sbc.Type = SPIRV;

			}
			fclose(fin);
			//system("del ./temp.spv");

			return sbc;
		}

		void AddShader(uint32_t pso, eastl::string filename, SHADER_KIND kind) {
			auto& p = psos.find(pso); assert(p != psos.end());
			p->second.shaders.push_back(LoadShaderByteCode(filename, kind, "main", GLSL));
		}

		void AddVertexInput(uint32_t pso, uint32_t binding, uint32_t stream, VkFormat format) {
			auto& p = psos.find(pso); assert(p != psos.end());
			VertexInput vi;
			vi.Binding = binding;
			vi.Stream = stream;
			vi.Format = format;
			p->second.VertexInputLayout.push_back(vi);
		}

		void SetPrimitiveTopology(uint32_t pso, VkPrimitiveTopology topology) {
			auto& p = psos.find(pso); assert(p != psos.end());
			p->second.InputAssemblyInfo.topology = topology;
		}

		void AddAttachmentInfo(uint32_t pso, bool blendEnable,
			VkBlendFactor srcColor, VkBlendFactor dstColor, VkBlendOp colorOp,
			VkBlendFactor srcAlpha, VkBlendFactor dstAlpha, VkBlendOp alphaOp) {
			auto& p = psos.find(pso); assert(p != psos.end());

			VkPipelineColorBlendAttachmentState state = GetDefaultColorBlendAttachmentState();
			state.blendEnable = blendEnable;
			state.srcColorBlendFactor = srcColor;
			state.dstColorBlendFactor = dstColor;
			state.colorBlendOp = colorOp;
			state.srcAlphaBlendFactor = srcAlpha;
			state.dstAlphaBlendFactor = dstAlpha;
			state.alphaBlendOp = alphaOp;
			p->second.AttachmentInfos.push_back(state);
		}

		void AddDefaultAttachmentInfo(uint32_t pso) {
			auto& p = psos.find(pso); assert(p != psos.end());
			p->second.AttachmentInfos.push_back(GetDefaultColorBlendAttachmentState());
		}

		void SetRasterStateInfo(uint32_t pso, VkCullModeFlags cullMode, VkFrontFace frontFace, bool depthClampEnable,
			bool discardEnable, VkPolygonMode polygonMode, bool depthBiasEnable, float depthBiasConstant, float depthBiasClamp, float depthBiasSlope, float lineWidth) {
			auto& p = psos.find(pso); assert(p != psos.end());
			VkPipelineRasterizationStateCreateInfo state = GetDefaultRasterState();
			state.cullMode = cullMode;
			state.frontFace = frontFace;
			state.depthClampEnable = depthClampEnable;
			state.rasterizerDiscardEnable = discardEnable;
			state.polygonMode = polygonMode;
			state.depthBiasEnable = depthBiasEnable;
			state.depthBiasConstantFactor = depthBiasConstant;
			state.depthBiasClamp = depthBiasClamp;
			state.depthBiasSlopeFactor = depthBiasSlope;
			state.lineWidth = lineWidth;
			state.depthBiasClamp = depthBiasClamp;
			p->second.RasterInfo = state;
		}

		void SetDepthStencilStateInfo(uint32_t pso, bool depthTestEnable, bool depthWriteEnable, uint32_t stencilMask) {
			auto& p = psos.find(pso); assert(p != psos.end());
			VkPipelineDepthStencilStateCreateInfo state = GetDefaultDepthStencilstate();
			state.depthTestEnable = depthTestEnable;
			state.depthWriteEnable = depthWriteEnable;
			state.front.writeMask = stencilMask;
			p->second.DepthStencilInfo = state;
		}

		void SetRenderPass(uint32_t pso, eastl::string renderPass) {
			auto& p = psos.find(pso); assert(p != psos.end());
			p->second.RenderPass = HashString(renderPass);
		}

		ASSET_DLL void InitInterface() {
			AngelScript::asIScriptEngine* engine = g_ScriptEngine.GetEngine();

			int r = 0;
			//Register our engine functions
			engine->RegisterEnum("SHADER_KIND");
			engine->RegisterEnumValue("SHADER_KIND", "VERTEX", SHADER_KIND::VERTEX);
			engine->RegisterEnumValue("SHADER_KIND", "FRAGMENT", SHADER_KIND::FRAGMENT);
			engine->RegisterEnumValue("SHADER_KIND", "GEOMETRY", SHADER_KIND::GEOMETRY);
			engine->RegisterEnumValue("SHADER_KIND", "EVALUATION", SHADER_KIND::EVALUATION);
			engine->RegisterEnumValue("SHADER_KIND", "CONTROL", SHADER_KIND::CONTROL);
			engine->RegisterEnumValue("SHADER_KIND", "COMPUTE", SHADER_KIND::COMPUTE);
			engine->RegisterEnumValue("SHADER_KIND", "MESH", SHADER_KIND::MESH);
			engine->RegisterEnumValue("SHADER_KIND", "TASK", SHADER_KIND::TASK);
			engine->RegisterEnumValue("SHADER_KIND", "RAY_GEN", SHADER_KIND::RAY_GEN);
			engine->RegisterEnumValue("SHADER_KIND", "RAY_INTERSECTION", SHADER_KIND::RAY_INTERSECTION);
			engine->RegisterEnumValue("SHADER_KIND", "RAY_ANY_HIT", SHADER_KIND::RAY_ANY_HIT);
			engine->RegisterEnumValue("SHADER_KIND", "RAY_MISS", SHADER_KIND::RAY_MISS);
			engine->RegisterEnumValue("SHADER_KIND", "RAY_CALLABLE", SHADER_KIND::RAY_CALLABLE);

			r = engine->RegisterGlobalFunction("uint CreatePSO(string name)", AngelScript::asFUNCTION(CreatePSO), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void AddShader(uint pso, string filename, SHADER_KIND kind)", AngelScript::asFUNCTION(AddShader), AngelScript::asCALL_CDECL); assert(r >= 0);
			
			//Register vulkan enums
			engine->RegisterEnum("DescriptorType");
			engine->RegisterEnumValue("DescriptorType", "Sampler", VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER);
			engine->RegisterEnumValue("DescriptorType", "CombinedImageSampler", VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
			engine->RegisterEnumValue("DescriptorType", "SampledImage", VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
			engine->RegisterEnumValue("DescriptorType", "StorageImage", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			engine->RegisterEnumValue("DescriptorType", "UniformTexelBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER);
			engine->RegisterEnumValue("DescriptorType", "StorageTexelBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER);
			engine->RegisterEnumValue("DescriptorType", "UniformBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			engine->RegisterEnumValue("DescriptorType", "StorageBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
			engine->RegisterEnumValue("DescriptorType", "UniformBufferDynamic", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
			engine->RegisterEnumValue("DescriptorType", "StorageBufferDynamic", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
			engine->RegisterEnumValue("DescriptorType", "InputAttachment", VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT);

			engine->RegisterEnum("BlendFactor");
			engine->RegisterEnumValue("BlendFactor", "Zero", VkBlendFactor::VK_BLEND_FACTOR_ZERO);
			engine->RegisterEnumValue("BlendFactor", "One", VkBlendFactor::VK_BLEND_FACTOR_ONE);
			engine->RegisterEnumValue("BlendFactor", "SrcColor", VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR);
			engine->RegisterEnumValue("BlendFactor", "OneMinusSrcColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR);
			engine->RegisterEnumValue("BlendFactor", "DstColor", VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR);
			engine->RegisterEnumValue("BlendFactor", "OneMinusDstColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR);
			engine->RegisterEnumValue("BlendFactor", "OneMinusSrcAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "DstAlpha", VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "OneMinusDstAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "ConstantColor", VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR);
			engine->RegisterEnumValue("BlendFactor", "OneMinusConstantColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR);
			engine->RegisterEnumValue("BlendFactor", "ConstantAlpha", VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "OneMinusConstantAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "SrcAlphaSaturate", VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE);
			engine->RegisterEnumValue("BlendFactor", "Src1Color", VkBlendFactor::VK_BLEND_FACTOR_SRC1_COLOR);
			engine->RegisterEnumValue("BlendFactor", "OneMinusSrc1Color", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR);
			engine->RegisterEnumValue("BlendFactor", "Src1Alpha", VkBlendFactor::VK_BLEND_FACTOR_SRC1_ALPHA);
			engine->RegisterEnumValue("BlendFactor", "OneMinusSrc1Alpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);

			engine->RegisterEnum("BlendOp");
			engine->RegisterEnumValue("BlendOp", "Add", VkBlendOp::VK_BLEND_OP_ADD);
			engine->RegisterEnumValue("BlendOp", "Subtract", VkBlendOp::VK_BLEND_OP_SUBTRACT);
			engine->RegisterEnumValue("BlendOp", "ReverseSubtract", VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT);
			engine->RegisterEnumValue("BlendOp", "Min", VkBlendOp::VK_BLEND_OP_MIN);
			engine->RegisterEnumValue("BlendOp", "Max", VkBlendOp::VK_BLEND_OP_MAX);

			engine->RegisterEnum("LogicOp");
			engine->RegisterEnumValue("LogicOp", "Clear", VkLogicOp::VK_LOGIC_OP_CLEAR);
			engine->RegisterEnumValue("LogicOp", "And", VkLogicOp::VK_LOGIC_OP_AND);
			engine->RegisterEnumValue("LogicOp", "AndReverse", VkLogicOp::VK_LOGIC_OP_AND_REVERSE);
			engine->RegisterEnumValue("LogicOp", "Copy", VkLogicOp::VK_LOGIC_OP_COPY);
			engine->RegisterEnumValue("LogicOp", "AndInverted", VkLogicOp::VK_LOGIC_OP_AND_INVERTED);
			engine->RegisterEnumValue("LogicOp", "NoOp", VkLogicOp::VK_LOGIC_OP_NO_OP);
			engine->RegisterEnumValue("LogicOp", "Xor", VkLogicOp::VK_LOGIC_OP_XOR);
			engine->RegisterEnumValue("LogicOp", "Or", VkLogicOp::VK_LOGIC_OP_OR);
			engine->RegisterEnumValue("LogicOp", "Nor", VkLogicOp::VK_LOGIC_OP_NOR);
			engine->RegisterEnumValue("LogicOp", "Equivalent", VkLogicOp::VK_LOGIC_OP_EQUIVALENT);
			engine->RegisterEnumValue("LogicOp", "Invert", VkLogicOp::VK_LOGIC_OP_INVERT);
			engine->RegisterEnumValue("LogicOp", "OrReverse", VkLogicOp::VK_LOGIC_OP_OR_REVERSE);
			engine->RegisterEnumValue("LogicOp", "CopyInverted", VkLogicOp::VK_LOGIC_OP_COPY_INVERTED);
			engine->RegisterEnumValue("LogicOp", "OrInverted", VkLogicOp::VK_LOGIC_OP_OR_INVERTED);
			engine->RegisterEnumValue("LogicOp", "Nand", VkLogicOp::VK_LOGIC_OP_NAND);
			engine->RegisterEnumValue("LogicOp", "Set", VkLogicOp::VK_LOGIC_OP_SET);

			engine->RegisterEnum("StencilOp");
			engine->RegisterEnumValue("StencilOp", "Never", VkCompareOp::VK_COMPARE_OP_NEVER);
			engine->RegisterEnumValue("StencilOp", "Less", VkCompareOp::VK_COMPARE_OP_LESS);
			engine->RegisterEnumValue("StencilOp", "Equal", VkCompareOp::VK_COMPARE_OP_EQUAL);
			engine->RegisterEnumValue("StencilOp", "LessOrEqual", VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL);
			engine->RegisterEnumValue("StencilOp", "Greater", VkCompareOp::VK_COMPARE_OP_GREATER);
			engine->RegisterEnumValue("StencilOp", "NotEqual", VkCompareOp::VK_COMPARE_OP_NOT_EQUAL);
			engine->RegisterEnumValue("StencilOp", "GreaterOrEqual", VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL);
			engine->RegisterEnumValue("StencilOp", "Always", VkCompareOp::VK_COMPARE_OP_ALWAYS);

			engine->RegisterEnum("PrimitiveTopology");
			engine->RegisterEnumValue("PrimitiveTopology", "PointList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
			engine->RegisterEnumValue("PrimitiveTopology", "LineList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
			engine->RegisterEnumValue("PrimitiveTopology", "LineStrip", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);
			engine->RegisterEnumValue("PrimitiveTopology", "TriangleList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
			engine->RegisterEnumValue("PrimitiveTopology", "TriangleStrip", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
			engine->RegisterEnumValue("PrimitiveTopology", "TriangleFan", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN);
			engine->RegisterEnumValue("PrimitiveTopology", "LineListWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY);
			engine->RegisterEnumValue("PrimitiveTopology", "LineStripWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY);
			engine->RegisterEnumValue("PrimitiveTopology", "TriangleListWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY);
			engine->RegisterEnumValue("PrimitiveTopology", "TriangleStripWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY);
			engine->RegisterEnumValue("PrimitiveTopology", "PatchList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST);
			
			engine->RegisterEnum("CullMode");
			engine->RegisterEnumValue("CullMode", "None", VkCullModeFlagBits::VK_CULL_MODE_NONE);
			engine->RegisterEnumValue("CullMode", "Front", VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT);
			engine->RegisterEnumValue("CullMode", "Back", VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT);
			engine->RegisterEnumValue("CullMode", "FrontAndBack", VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK);
		
			engine->RegisterEnum("FrontFace");
			engine->RegisterEnumValue("FrontFace", "CounterClockwise", VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE);
			engine->RegisterEnumValue("FrontFace", "Clockwise", VkFrontFace::VK_FRONT_FACE_CLOCKWISE);
			
			engine->RegisterEnum("PolygonMode");
			engine->RegisterEnumValue("PolygonMode", "Fill", VkPolygonMode::VK_POLYGON_MODE_FILL);
			engine->RegisterEnumValue("PolygonMode", "Line", VkPolygonMode::VK_POLYGON_MODE_LINE);
			engine->RegisterEnumValue("PolygonMode", "Point", VkPolygonMode::VK_POLYGON_MODE_POINT);
			
			engine->RegisterEnum("VertexInputRate");
			engine->RegisterEnumValue("VertexInputRate", "Instance", VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE);
			engine->RegisterEnumValue("VertexInputRate", "Vertex", VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX);
		
			engine->RegisterEnum("Format");
			engine->RegisterEnumValue("Format", "Undefined", VkFormat::VK_FORMAT_UNDEFINED);
			engine->RegisterEnumValue("Format", "R4G4UnormPack8", VkFormat::VK_FORMAT_R4G4_UNORM_PACK8);
			engine->RegisterEnumValue("Format", "R4G4B4A4UnormPack16", VkFormat::VK_FORMAT_R4G4B4A4_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "B4G4R4A4UnormPack16", VkFormat::VK_FORMAT_B4G4R4A4_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "R5G6B5UnormPack16", VkFormat::VK_FORMAT_R5G6B5_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "B5G6R5UnormPack16", VkFormat::VK_FORMAT_B5G6R5_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "R5G5B5A1UnormPack16", VkFormat::VK_FORMAT_R5G5B5A1_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "B5G5R5A1UnormPack16", VkFormat::VK_FORMAT_B5G5R5A1_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "A1R5G5B5UnormPack16", VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16);
			engine->RegisterEnumValue("Format", "R8Unorm", VkFormat::VK_FORMAT_R8_UNORM);
			engine->RegisterEnumValue("Format", "R8Snorm", VkFormat::VK_FORMAT_R8_SNORM);
			engine->RegisterEnumValue("Format", "R8Uscaled", VkFormat::VK_FORMAT_R8_USCALED);
			engine->RegisterEnumValue("Format", "R8Sscaled", VkFormat::VK_FORMAT_R8_SSCALED);
			engine->RegisterEnumValue("Format", "R8Uint", VkFormat::VK_FORMAT_R8_UINT);
			engine->RegisterEnumValue("Format", "R8Sint", VkFormat::VK_FORMAT_R8_SINT);
			engine->RegisterEnumValue("Format", "R8Srgb", VkFormat::VK_FORMAT_R8_SRGB);
			engine->RegisterEnumValue("Format", "R8G8Unorm", VkFormat::VK_FORMAT_R8G8_UNORM);
			engine->RegisterEnumValue("Format", "R8G8Snorm", VkFormat::VK_FORMAT_R8G8_SNORM);
			engine->RegisterEnumValue("Format", "R8G8Uscaled", VkFormat::VK_FORMAT_R8G8_USCALED);
			engine->RegisterEnumValue("Format", "R8G8Sscaled", VkFormat::VK_FORMAT_R8G8_SSCALED);
			engine->RegisterEnumValue("Format", "R8G8Uint", VkFormat::VK_FORMAT_R8G8_UINT);
			engine->RegisterEnumValue("Format", "R8G8Sint", VkFormat::VK_FORMAT_R8G8_SINT);
			engine->RegisterEnumValue("Format", "R8G8Srgb", VkFormat::VK_FORMAT_R8G8_SRGB);
			engine->RegisterEnumValue("Format", "R8G8B8Unorm", VkFormat::VK_FORMAT_R8G8B8_UNORM);
			engine->RegisterEnumValue("Format", "R8G8B8Snorm", VkFormat::VK_FORMAT_R8G8B8_SNORM);
			engine->RegisterEnumValue("Format", "R8G8B8Uscaled", VkFormat::VK_FORMAT_R8G8B8_USCALED);
			engine->RegisterEnumValue("Format", "R8G8B8Sscaled", VkFormat::VK_FORMAT_R8G8B8_SSCALED);
			engine->RegisterEnumValue("Format", "R8G8B8Uint", VkFormat::VK_FORMAT_R8G8B8_UINT);
			engine->RegisterEnumValue("Format", "R8G8B8Sint", VkFormat::VK_FORMAT_R8G8B8_SINT);
			engine->RegisterEnumValue("Format", "R8G8B8Srgb", VkFormat::VK_FORMAT_R8G8B8_SRGB);
			engine->RegisterEnumValue("Format", "B8G8R8Unorm", VkFormat::VK_FORMAT_B8G8R8_UNORM );
			engine->RegisterEnumValue("Format", "B8G8R8Snorm", VkFormat::VK_FORMAT_B8G8R8_SNORM );
			engine->RegisterEnumValue("Format", "B8G8R8Uscaled", VkFormat::VK_FORMAT_B8G8R8_USCALED);
			engine->RegisterEnumValue("Format", "B8G8R8Sscaled", VkFormat::VK_FORMAT_B8G8R8_SSCALED);
			engine->RegisterEnumValue("Format", "B8G8R8Uint", VkFormat::VK_FORMAT_B8G8R8_UINT);
			engine->RegisterEnumValue("Format", "B8G8R8Sint", VkFormat::VK_FORMAT_B8G8R8_SINT);
			engine->RegisterEnumValue("Format", "B8G8R8Srgb", VkFormat::VK_FORMAT_B8G8R8_SRGB);
			engine->RegisterEnumValue("Format", "R8G8B8A8Unorm", VkFormat::VK_FORMAT_R8G8B8A8_UNORM);
			engine->RegisterEnumValue("Format", "R8G8B8A8Snorm", VkFormat::VK_FORMAT_R8G8B8A8_SNORM);
			engine->RegisterEnumValue("Format", "R8G8B8A8Uscaled", VkFormat::VK_FORMAT_R8G8B8A8_USCALED);
			engine->RegisterEnumValue("Format", "R8G8B8A8Sscaled", VkFormat::VK_FORMAT_R8G8B8A8_SSCALED);
			engine->RegisterEnumValue("Format", "R8G8B8A8Uint", VkFormat::VK_FORMAT_R8G8B8A8_UINT);
			engine->RegisterEnumValue("Format", "R8G8B8A8Sint", VkFormat::VK_FORMAT_R8G8B8A8_SINT);
			engine->RegisterEnumValue("Format", "R8G8B8A8Srgb", VkFormat::VK_FORMAT_R8G8B8A8_SRGB);
			engine->RegisterEnumValue("Format", "B8G8R8A8Unorm", VkFormat::VK_FORMAT_B8G8R8A8_UNORM);
			engine->RegisterEnumValue("Format", "B8G8R8A8Snorm", VkFormat::VK_FORMAT_B8G8R8A8_SNORM);
			engine->RegisterEnumValue("Format", "B8G8R8A8Uscaled", VkFormat::VK_FORMAT_B8G8R8A8_USCALED);
			engine->RegisterEnumValue("Format", "B8G8R8A8Sscaled", VkFormat::VK_FORMAT_B8G8R8A8_SSCALED);
			engine->RegisterEnumValue("Format", "B8G8R8A8Uint", VkFormat::VK_FORMAT_B8G8R8A8_UINT);
			engine->RegisterEnumValue("Format", "B8G8R8A8Sint", VkFormat::VK_FORMAT_B8G8R8A8_SINT);
			engine->RegisterEnumValue("Format", "B8G8R8A8Srgb", VkFormat::VK_FORMAT_B8G8R8A8_SRGB);
			engine->RegisterEnumValue("Format", "A8B8G8R8UnormPack32", VkFormat::VK_FORMAT_A8B8G8R8_UNORM_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8SnormPack32", VkFormat::VK_FORMAT_A8B8G8R8_SNORM_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8UscaledPack32", VkFormat::VK_FORMAT_A8B8G8R8_USCALED_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8SscaledPack32", VkFormat::VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8UintPack32", VkFormat::VK_FORMAT_A8B8G8R8_UINT_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8SintPack32", VkFormat::VK_FORMAT_A8B8G8R8_SINT_PACK32);
			engine->RegisterEnumValue("Format", "A8B8G8R8SrgbPack32", VkFormat::VK_FORMAT_A8B8G8R8_SRGB_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10UnormPack32", VkFormat::VK_FORMAT_A2R10G10B10_UNORM_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10SnormPack32", VkFormat::VK_FORMAT_A2R10G10B10_SNORM_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10UscaledPack32", VkFormat::VK_FORMAT_A2R10G10B10_USCALED_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10SscaledPack32", VkFormat::VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10UintPack32", VkFormat::VK_FORMAT_A2R10G10B10_UINT_PACK32);
			engine->RegisterEnumValue("Format", "A2R10G10B10SintPack32", VkFormat::VK_FORMAT_A2R10G10B10_SINT_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10UnormPack32", VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10SnormPack32", VkFormat::VK_FORMAT_A2B10G10R10_SNORM_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10UscaledPack32", VkFormat::VK_FORMAT_A2B10G10R10_USCALED_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10SscaledPack32", VkFormat::VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10UintPack32", VkFormat::VK_FORMAT_A2B10G10R10_UINT_PACK32);
			engine->RegisterEnumValue("Format", "A2B10G10R10SintPack32", VkFormat::VK_FORMAT_A2B10G10R10_SINT_PACK32);
			engine->RegisterEnumValue("Format", "R16Unorm", VkFormat::VK_FORMAT_R16_UNORM);
			engine->RegisterEnumValue("Format", "R16Snorm", VkFormat::VK_FORMAT_R16_SNORM);
			engine->RegisterEnumValue("Format", "R16Uscaled", VkFormat::VK_FORMAT_R16_USCALED);
			engine->RegisterEnumValue("Format", "R16Sscaled", VkFormat::VK_FORMAT_R16_SSCALED);
			engine->RegisterEnumValue("Format", "R16Uint", VkFormat::VK_FORMAT_R16_UINT);
			engine->RegisterEnumValue("Format", "R16Sint", VkFormat::VK_FORMAT_R16_SINT);
			engine->RegisterEnumValue("Format", "R16Sfloat", VkFormat::VK_FORMAT_R16_SFLOAT);
			engine->RegisterEnumValue("Format", "R16G16Unorm", VkFormat::VK_FORMAT_R16G16_UNORM);
			engine->RegisterEnumValue("Format", "R16G16Snorm", VkFormat::VK_FORMAT_R16G16_SNORM);
			engine->RegisterEnumValue("Format", "R16G16Uscaled", VkFormat::VK_FORMAT_R16G16_USCALED);
			engine->RegisterEnumValue("Format", "R16G16Sscaled", VkFormat::VK_FORMAT_R16G16_SSCALED);
			engine->RegisterEnumValue("Format", "R16G16Uint", VkFormat::VK_FORMAT_R16G16_UINT);
			engine->RegisterEnumValue("Format", "R16G16Sint", VkFormat::VK_FORMAT_R16G16_SINT);
			engine->RegisterEnumValue("Format", "R16G16Sfloat", VkFormat::VK_FORMAT_R16G16_SFLOAT);
			engine->RegisterEnumValue("Format", "R16G16B16Unorm", VkFormat::VK_FORMAT_R16G16B16_UNORM);
			engine->RegisterEnumValue("Format", "R16G16B16Snorm", VkFormat::VK_FORMAT_R16G16B16_SNORM);
			engine->RegisterEnumValue("Format", "R16G16B16Uscaled", VkFormat::VK_FORMAT_R16G16B16_USCALED);
			engine->RegisterEnumValue("Format", "R16G16B16Sscaled", VkFormat::VK_FORMAT_R16G16B16_SSCALED);
			engine->RegisterEnumValue("Format", "R16G16B16Uint", VkFormat::VK_FORMAT_R16G16B16_UINT);
			engine->RegisterEnumValue("Format", "R16G16B16Sint", VkFormat::VK_FORMAT_R16G16B16_SINT);
			engine->RegisterEnumValue("Format", "R16G16B16Sfloat", VkFormat::VK_FORMAT_R16G16B16_SFLOAT);
			engine->RegisterEnumValue("Format", "R16G16B16A16Unorm", VkFormat::VK_FORMAT_R16G16B16A16_UNORM);
			engine->RegisterEnumValue("Format", "R16G16B16A16Snorm", VkFormat::VK_FORMAT_R16G16B16A16_SNORM);
			engine->RegisterEnumValue("Format", "R16G16B16A16Uscaled", VkFormat::VK_FORMAT_R16G16B16A16_USCALED);
			engine->RegisterEnumValue("Format", "R16G16B16A16Sscaled", VkFormat::VK_FORMAT_R16G16B16A16_SSCALED);
			engine->RegisterEnumValue("Format", "R16G16B16A16Uint", VkFormat::VK_FORMAT_R16G16B16A16_UINT);
			engine->RegisterEnumValue("Format", "R16G16B16A16Sint", VkFormat::VK_FORMAT_R16G16B16A16_SINT);
			engine->RegisterEnumValue("Format", "R16G16B16A16Sfloat", VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT);
			engine->RegisterEnumValue("Format", "R32Uint", VkFormat::VK_FORMAT_R32_UINT);
			engine->RegisterEnumValue("Format", "R32Sint", VkFormat::VK_FORMAT_R32_SINT);
			engine->RegisterEnumValue("Format", "R32Sfloat", VkFormat::VK_FORMAT_R32_SFLOAT);
			engine->RegisterEnumValue("Format", "R32G32Uint", VkFormat::VK_FORMAT_R32G32_UINT);
			engine->RegisterEnumValue("Format", "R32G32Sint", VkFormat::VK_FORMAT_R32G32_SINT);
			engine->RegisterEnumValue("Format", "R32G32Sfloat", VkFormat::VK_FORMAT_R32G32_SFLOAT);
			engine->RegisterEnumValue("Format", "R32G32B32Uint", VkFormat::VK_FORMAT_R32G32B32_UINT);
			engine->RegisterEnumValue("Format", "R32G32B32Sint", VkFormat::VK_FORMAT_R32G32B32_SINT);
			engine->RegisterEnumValue("Format", "R32G32B32Sfloat", VkFormat::VK_FORMAT_R32G32B32_SFLOAT);
			engine->RegisterEnumValue("Format", "R32G32B32A32Uint", VkFormat::VK_FORMAT_R32G32B32A32_UINT);
			engine->RegisterEnumValue("Format", "R32G32B32A32Sint", VkFormat::VK_FORMAT_R32G32B32A32_SINT);
			engine->RegisterEnumValue("Format", "R32G32B32A32Sfloat", VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT);
			engine->RegisterEnumValue("Format", "R64Uint", VkFormat::VK_FORMAT_R64_UINT);
			engine->RegisterEnumValue("Format", "R64Sint", VkFormat::VK_FORMAT_R64_SINT);
			engine->RegisterEnumValue("Format", "R64Sfloat", VkFormat::VK_FORMAT_R64_SFLOAT);
			engine->RegisterEnumValue("Format", "R64G64Uint", VkFormat::VK_FORMAT_R64G64_UINT);
			engine->RegisterEnumValue("Format", "R64G64Sint", VkFormat::VK_FORMAT_R64G64_SINT);
			engine->RegisterEnumValue("Format", "R64G64Sfloat", VkFormat::VK_FORMAT_R64G64_SFLOAT);
			engine->RegisterEnumValue("Format", "R64G64B64Uint", VkFormat::VK_FORMAT_R64G64B64_UINT);
			engine->RegisterEnumValue("Format", "R64G64B64Sint", VkFormat::VK_FORMAT_R64G64B64_SINT);
			engine->RegisterEnumValue("Format", "R64G64B64Sfloat", VkFormat::VK_FORMAT_R64G64B64_SFLOAT);
			engine->RegisterEnumValue("Format", "R64G64B64A64Uint", VkFormat::VK_FORMAT_R64G64B64A64_UINT);
			engine->RegisterEnumValue("Format", "R64G64B64A64Sint", VkFormat::VK_FORMAT_R64G64B64A64_SINT);
			engine->RegisterEnumValue("Format", "R64G64B64A64Sfloat", VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT);
			engine->RegisterEnumValue("Format", "B10G11R11UfloatPack32", VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32);
			engine->RegisterEnumValue("Format", "E5B9G9R9UfloatPack32", VkFormat::VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
			engine->RegisterEnumValue("Format", "D16Unorm", VkFormat::VK_FORMAT_D16_UNORM);
			engine->RegisterEnumValue("Format", "X8D24UnormPack32", VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32);
			engine->RegisterEnumValue("Format", "D32Sfloat", VkFormat::VK_FORMAT_D32_SFLOAT);
			engine->RegisterEnumValue("Format", "S8Uint", VkFormat::VK_FORMAT_S8_UINT);
			engine->RegisterEnumValue("Format", "D16UnormS8Uint", VkFormat::VK_FORMAT_D16_UNORM_S8_UINT);
			engine->RegisterEnumValue("Format", "D24UnormS8Uint", VkFormat::VK_FORMAT_D24_UNORM_S8_UINT);
			engine->RegisterEnumValue("Format", "D32SfloatS8Uint", VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT);
			engine->RegisterEnumValue("Format", "Bc1RgbUnormBlock", VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc1RgbSrgbBlock", VkFormat::VK_FORMAT_BC1_RGB_SRGB_BLOCK);
			engine->RegisterEnumValue("Format", "Bc1RgbaUnormBlock", VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc1RgbaSrgbBlock", VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
			engine->RegisterEnumValue("Format", "Bc2UnormBlock", VkFormat::VK_FORMAT_BC2_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc2SrgbBlock", VkFormat::VK_FORMAT_BC2_SRGB_BLOCK);
			engine->RegisterEnumValue("Format", "Bc3UnormBlock", VkFormat::VK_FORMAT_BC3_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc3SrgbBlock", VkFormat::VK_FORMAT_BC3_SRGB_BLOCK);
			engine->RegisterEnumValue("Format", "Bc4UnormBlock", VkFormat::VK_FORMAT_BC4_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc4SnormBlock", VkFormat::VK_FORMAT_BC4_SNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc5UnormBlock", VkFormat::VK_FORMAT_BC5_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc5SnormBlock", VkFormat::VK_FORMAT_BC5_SNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc6HUfloatBlock", VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK);
			engine->RegisterEnumValue("Format", "Bc6HSfloatBlock", VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK);
			engine->RegisterEnumValue("Format", "Bc7UnormBlock", VkFormat::VK_FORMAT_BC7_UNORM_BLOCK);
			engine->RegisterEnumValue("Format", "Bc7SrgbBlock", VkFormat::VK_FORMAT_BC7_SRGB_BLOCK);

			r = engine->RegisterGlobalFunction("void SetPrimitiveTopology(uint pso, PrimitiveTopology topology)", AngelScript::asFUNCTION(SetPrimitiveTopology), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void AddAttachmentInfo(uint pso, bool blendEnable, BlendFactor srcColor, BlendFactor dstColor, BlendOp colorOp, BlendFactor srcAlpha, BlendFactor dstAlpha, BlendOp alphaOp)", AngelScript::asFUNCTION(AddAttachmentInfo), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void AddDefaultAttachmentInfo(uint pso)", AngelScript::asFUNCTION(AddDefaultAttachmentInfo), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void SetRasterStateInfo(uint pso, CullMode cullMode, FrontFace frontFace, bool depthClampEnable, bool discardEnable, PolygonMode polygonMode, bool depthBiasEnable, float depthBiasConstant, float depthBiasClamp, float depthBiasSlope, float lineWidth)", AngelScript::asFUNCTION(SetRasterStateInfo), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void SetDepthStencilStateInfo(uint pso, bool depthTestEnable, bool depthWriteEnable, uint stencilMask)", AngelScript::asFUNCTION(SetDepthStencilStateInfo), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void AddVertexInput(uint pso, uint binding, uint stream, Format format)", AngelScript::asFUNCTION(AddVertexInput), AngelScript::asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void SetRenderPass(uint pso, string binding)", AngelScript::asFUNCTION(SetRenderPass), AngelScript::asCALL_CDECL); assert(r >= 0);
		}

		ScriptPipelineState* GetPSO(uint32_t hash) {
			auto& p = psos.find(hash);
			if (p == psos.end()) {
				return nullptr;
			}
			return &p->second;
		}
	}
}