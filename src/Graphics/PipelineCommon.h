#pragma once
#include "volk.h"
namespace smug {
#pragma region mappings
//static mappings to vulkan enums
static eastl::unordered_map<eastl::string, VkDescriptorType> ToDescriptorType = {
	{ "Sampler", VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER },
	{ "CombinedImageSampler", VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER },
	{ "SampledImage", VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE },
	{ "StorageImage", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
	{ "UniformTexelBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER },
	{ "StorageTexelBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER },
	{ "UniformBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
	{ "StorageBuffer", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER },
	{ "UniformBufferDynamic", VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC },
	{ "StorageBufferDynamic", VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC },
	{ "InputAttachment", VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT }
};

static eastl::unordered_map<eastl::string, VkBlendFactor> ToBlendFactor = {
	{ "Zero", VkBlendFactor::VK_BLEND_FACTOR_ZERO },
	{ "One", VkBlendFactor::VK_BLEND_FACTOR_ONE },
	{ "SrcColor", VkBlendFactor::VK_BLEND_FACTOR_SRC_COLOR },
	{ "OneMinusSrcColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR },
	{ "DstColor", VkBlendFactor::VK_BLEND_FACTOR_DST_COLOR },
	{ "OneMinusDstColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR },
	{ "OneMinusSrcAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA },
	{ "DstAlpha", VkBlendFactor::VK_BLEND_FACTOR_DST_ALPHA },
	{ "OneMinusDstAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA },
	{ "ConstantColor", VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_COLOR },
	{ "OneMinusConstantColor", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR },
	{ "ConstantAlpha", VkBlendFactor::VK_BLEND_FACTOR_CONSTANT_ALPHA },
	{ "OneMinusConstantAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA },
	{ "SrcAlphaSaturate", VkBlendFactor::VK_BLEND_FACTOR_SRC_ALPHA_SATURATE },
	{ "OneMinusConstantAlpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA },
	{ "Src1Color", VkBlendFactor::VK_BLEND_FACTOR_SRC1_COLOR },
	{ "OneMinusSrc1Color", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR },
	{ "Src1Alpha", VkBlendFactor::VK_BLEND_FACTOR_SRC1_ALPHA },
	{ "OneMinusSrc1Alpha", VkBlendFactor::VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA },
};

static eastl::unordered_map<eastl::string, VkBlendOp> ToBlendOp = {
	{ "Add", VkBlendOp::VK_BLEND_OP_ADD },
	{ "Subtract", VkBlendOp::VK_BLEND_OP_SUBTRACT },
	{ "ReverseSubtract", VkBlendOp::VK_BLEND_OP_REVERSE_SUBTRACT },
	{ "Min", VkBlendOp::VK_BLEND_OP_MIN },
	{ "Max", VkBlendOp::VK_BLEND_OP_MAX }
};

static eastl::unordered_map<int, VkShaderStageFlagBits> ToShaderStage = {
	{ 0, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT },
	{ 1, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT },
	{ 2, VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT },
	{ 3, VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
	{ 4, VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
	{ 5, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT }
};

static eastl::unordered_map<eastl::string, VkLogicOp> ToLogicOp = {
	{ "Clear", VkLogicOp::VK_LOGIC_OP_CLEAR },
	{ "And", VkLogicOp::VK_LOGIC_OP_AND },
	{ "AndReverse", VkLogicOp::VK_LOGIC_OP_AND_REVERSE },
	{ "Copy", VkLogicOp::VK_LOGIC_OP_COPY },
	{ "AndInverted", VkLogicOp::VK_LOGIC_OP_AND_INVERTED },
	{ "NoOp", VkLogicOp::VK_LOGIC_OP_NO_OP },
	{ "Xor", VkLogicOp::VK_LOGIC_OP_XOR },
	{ "Or", VkLogicOp::VK_LOGIC_OP_OR },
	{ "Nor", VkLogicOp::VK_LOGIC_OP_NOR },
	{ "Equivalent", VkLogicOp::VK_LOGIC_OP_EQUIVALENT },
	{ "Invert", VkLogicOp::VK_LOGIC_OP_INVERT },
	{ "OrReverse", VkLogicOp::VK_LOGIC_OP_OR_REVERSE },
	{ "CopyInverted", VkLogicOp::VK_LOGIC_OP_COPY_INVERTED },
	{ "OrInverted", VkLogicOp::VK_LOGIC_OP_OR_INVERTED },
	{ "Nand", VkLogicOp::VK_LOGIC_OP_NAND },
	{ "Set", VkLogicOp::VK_LOGIC_OP_SET }
};

static eastl::unordered_map<eastl::string, VkStencilOp> ToStencilOp = {
	{ "Keep", VkStencilOp::VK_STENCIL_OP_KEEP },
	{ "Zero", VkStencilOp::VK_STENCIL_OP_ZERO },
	{ "Replace", VkStencilOp::VK_STENCIL_OP_REPLACE },
	{ "IncrementAndClamp", VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_CLAMP },
	{ "DecrementAndClamp", VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_CLAMP },
	{ "Invert", VkStencilOp::VK_STENCIL_OP_INVERT },
	{ "IncrementAndWrap", VkStencilOp::VK_STENCIL_OP_INCREMENT_AND_WRAP },
	{ "DecrementAndWrap", VkStencilOp::VK_STENCIL_OP_DECREMENT_AND_WRAP }
};

static eastl::unordered_map<eastl::string, VkCompareOp> ToCompareOp = {
	{ "Never", VkCompareOp::VK_COMPARE_OP_NEVER },
	{ "Less", VkCompareOp::VK_COMPARE_OP_LESS },
	{ "Equal", VkCompareOp::VK_COMPARE_OP_EQUAL },
	{ "LessOrEqual", VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL },
	{ "Greater", VkCompareOp::VK_COMPARE_OP_GREATER },
	{ "NotEqual", VkCompareOp::VK_COMPARE_OP_NOT_EQUAL },
	{ "GreaterOrEqual", VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL },
	{ "Always", VkCompareOp::VK_COMPARE_OP_ALWAYS }
};

static eastl::unordered_map<eastl::string, VkPrimitiveTopology> ToPrimitiveTopology = {
	{ "PointList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_POINT_LIST },
	{ "LineList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST },
	{ "LineStrip", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP },
	{ "TriangleList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST },
	{ "TriangleStrip", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP },
	{ "TriangleFan", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN },
	{ "LineListWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY },
	{ "LineStripWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY },
	{ "TriangleListWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY },
	{ "TriangleStripWithAdjacency", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY },
	{ "PatchList", VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_PATCH_LIST }
};


static eastl::unordered_map<int, VkSampleCountFlagBits> ToSampleCount = {
	{ 1, VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT },
	{ 2, VkSampleCountFlagBits::VK_SAMPLE_COUNT_2_BIT },
	{ 4, VkSampleCountFlagBits::VK_SAMPLE_COUNT_4_BIT },
	{ 8, VkSampleCountFlagBits::VK_SAMPLE_COUNT_8_BIT },
	{ 16, VkSampleCountFlagBits::VK_SAMPLE_COUNT_16_BIT },
	{ 32, VkSampleCountFlagBits::VK_SAMPLE_COUNT_32_BIT },
	{ 64, VkSampleCountFlagBits::VK_SAMPLE_COUNT_64_BIT }
};

static eastl::unordered_map<eastl::string, VkCullModeFlagBits> ToCullMode = {
	{ "None", VkCullModeFlagBits::VK_CULL_MODE_NONE },
	{ "Front", VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT },
	{ "Back", VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT },
	{ "FrontAndBack", VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK }
};

static eastl::unordered_map<eastl::string, VkFrontFace> ToFrontFace = {
	{ "CounterClockwise", VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE },
	{ "Clockwise", VkFrontFace::VK_FRONT_FACE_CLOCKWISE }
};

static eastl::unordered_map<eastl::string, VkPolygonMode> ToPolygonMode = {
	{ "Fill", VkPolygonMode::VK_POLYGON_MODE_FILL },
	{ "Line", VkPolygonMode::VK_POLYGON_MODE_LINE },
	{ "Point", VkPolygonMode::VK_POLYGON_MODE_POINT }
};

static eastl::unordered_map<eastl::string, VkVertexInputRate> ToVertexInputRate = {
	{ "Instance", VkVertexInputRate::VK_VERTEX_INPUT_RATE_INSTANCE },
	{ "Vertex", VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX }
};

static eastl::unordered_map<eastl::string, VkFormat> ToFormat = {
	{ "Undefined", VkFormat::VK_FORMAT_UNDEFINED },
	{ "R4G4UnormPack8", VkFormat::VK_FORMAT_R4G4_UNORM_PACK8 },
	{ "R4G4B4A4UnormPack16", VkFormat::VK_FORMAT_R4G4B4A4_UNORM_PACK16 },
	{ "B4G4R4A4UnormPack16", VkFormat::VK_FORMAT_B4G4R4A4_UNORM_PACK16 },
	{ "R5G6B5UnormPack16", VkFormat::VK_FORMAT_R5G6B5_UNORM_PACK16 },
	{ "B5G6R5UnormPack16", VkFormat::VK_FORMAT_B5G6R5_UNORM_PACK16 },
	{ "R5G5B5A1UnormPack16", VkFormat::VK_FORMAT_R5G5B5A1_UNORM_PACK16 },
	{ "B5G5R5A1UnormPack16", VkFormat::VK_FORMAT_B5G5R5A1_UNORM_PACK16 },
	{ "A1R5G5B5UnormPack16", VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16 },
	{ "R8Unorm", VkFormat::VK_FORMAT_R8_UNORM },
	{ "R8Snorm", VkFormat::VK_FORMAT_R8_SNORM },
	{ "R8Uscaled", VkFormat::VK_FORMAT_R8_USCALED },
	{ "R8Sscaled", VkFormat::VK_FORMAT_R8_SSCALED },
	{ "R8Uint", VkFormat::VK_FORMAT_R8_UINT },
	{ "R8Sint", VkFormat::VK_FORMAT_R8_SINT },
	{ "R8Srgb", VkFormat::VK_FORMAT_R8_SRGB },
	{ "R8G8Unorm", VkFormat::VK_FORMAT_R8G8_UNORM },
	{ "R8G8Snorm", VkFormat::VK_FORMAT_R8G8_SNORM },
	{ "R8G8Uscaled", VkFormat::VK_FORMAT_R8G8_USCALED },
	{ "R8G8Sscaled", VkFormat::VK_FORMAT_R8G8_SSCALED },
	{ "R8G8Uint", VkFormat::VK_FORMAT_R8G8_UINT },
	{ "R8G8Sint", VkFormat::VK_FORMAT_R8G8_SINT },
	{ "R8G8Srgb", VkFormat::VK_FORMAT_R8G8_SRGB },
	{ "R8G8B8Unorm", VkFormat::VK_FORMAT_R8G8B8_UNORM },
	{ "R8G8B8Snorm", VkFormat::VK_FORMAT_R8G8B8_SNORM },
	{ "R8G8B8Uscaled", VkFormat::VK_FORMAT_R8G8B8_USCALED },
	{ "R8G8B8Sscaled", VkFormat::VK_FORMAT_R8G8B8_SSCALED },
	{ "R8G8B8Uint", VkFormat::VK_FORMAT_R8G8B8_UINT },
	{ "R8G8B8Sint", VkFormat::VK_FORMAT_R8G8B8_SINT },
	{ "R8G8B8Srgb", VkFormat::VK_FORMAT_R8G8B8_SRGB },
	{ "B8G8R8Unorm", VkFormat::VK_FORMAT_B8G8R8_UNORM },
	{ "B8G8R8Snorm", VkFormat::VK_FORMAT_B8G8R8_SNORM },
	{ "B8G8R8Uscaled", VkFormat::VK_FORMAT_B8G8R8_USCALED },
	{ "B8G8R8Sscaled", VkFormat::VK_FORMAT_B8G8R8_SSCALED },
	{ "B8G8R8Uint", VkFormat::VK_FORMAT_B8G8R8_UINT},
	{ "B8G8R8Sint", VkFormat::VK_FORMAT_B8G8R8_SINT },
	{ "B8G8R8Srgb", VkFormat::VK_FORMAT_B8G8R8_SRGB },
	{ "R8G8B8A8Unorm", VkFormat::VK_FORMAT_R8G8B8A8_UNORM },
	{ "R8G8B8A8Snorm", VkFormat::VK_FORMAT_R8G8B8A8_SNORM },
	{ "R8G8B8A8Uscaled", VkFormat::VK_FORMAT_R8G8B8A8_USCALED },
	{ "R8G8B8A8Sscaled", VkFormat::VK_FORMAT_R8G8B8A8_SSCALED },
	{ "R8G8B8A8Uint", VkFormat::VK_FORMAT_R8G8B8A8_UINT },
	{ "R8G8B8A8Sint", VkFormat::VK_FORMAT_R8G8B8A8_SINT },
	{ "R8G8B8A8Srgb", VkFormat::VK_FORMAT_R8G8B8A8_SRGB },
	{ "B8G8R8A8Unorm", VkFormat::VK_FORMAT_B8G8R8A8_UNORM },
	{ "B8G8R8A8Snorm", VkFormat::VK_FORMAT_B8G8R8A8_SNORM },
	{ "B8G8R8A8Uscaled", VkFormat::VK_FORMAT_B8G8R8A8_USCALED },
	{ "B8G8R8A8Sscaled", VkFormat::VK_FORMAT_B8G8R8A8_SSCALED },
	{ "B8G8R8A8Uint", VkFormat::VK_FORMAT_B8G8R8A8_UINT },
	{ "B8G8R8A8Sint", VkFormat::VK_FORMAT_B8G8R8A8_SINT },
	{ "B8G8R8A8Srgb", VkFormat::VK_FORMAT_B8G8R8A8_SRGB },
	{ "A8B8G8R8UnormPack32", VkFormat::VK_FORMAT_A8B8G8R8_UNORM_PACK32 },
	{ "A8B8G8R8SnormPack32", VkFormat::VK_FORMAT_A8B8G8R8_SNORM_PACK32 },
	{ "A8B8G8R8UscaledPack32", VkFormat::VK_FORMAT_A8B8G8R8_USCALED_PACK32 },
	{ "A8B8G8R8SscaledPack32", VkFormat::VK_FORMAT_A8B8G8R8_SSCALED_PACK32 },
	{ "A8B8G8R8UintPack32", VkFormat::VK_FORMAT_A8B8G8R8_UINT_PACK32 },
	{ "A8B8G8R8SintPack32", VkFormat::VK_FORMAT_A8B8G8R8_SINT_PACK32 },
	{ "A8B8G8R8SrgbPack32", VkFormat::VK_FORMAT_A8B8G8R8_SRGB_PACK32 },
	{ "A2R10G10B10UnormPack32", VkFormat::VK_FORMAT_A2R10G10B10_UNORM_PACK32 },
	{ "A2R10G10B10SnormPack32", VkFormat::VK_FORMAT_A2R10G10B10_SNORM_PACK32 },
	{ "A2R10G10B10UscaledPack32", VkFormat::VK_FORMAT_A2R10G10B10_USCALED_PACK32 },
	{ "A2R10G10B10SscaledPack32", VkFormat::VK_FORMAT_A2R10G10B10_SSCALED_PACK32 },
	{ "A2R10G10B10UintPack32", VkFormat::VK_FORMAT_A2R10G10B10_UINT_PACK32 },
	{ "A2R10G10B10SintPack32", VkFormat::VK_FORMAT_A2R10G10B10_SINT_PACK32 },
	{ "A2B10G10R10UnormPack32", VkFormat::VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
	{ "A2B10G10R10SnormPack32", VkFormat::VK_FORMAT_A2B10G10R10_SNORM_PACK32 },
	{ "A2B10G10R10UscaledPack32", VkFormat::VK_FORMAT_A2B10G10R10_USCALED_PACK32 },
	{ "A2B10G10R10SscaledPack32", VkFormat::VK_FORMAT_A2B10G10R10_SSCALED_PACK32 },
	{ "A2B10G10R10UintPack32", VkFormat::VK_FORMAT_A2B10G10R10_UINT_PACK32 },
	{ "A2B10G10R10SintPack32", VkFormat::VK_FORMAT_A2B10G10R10_SINT_PACK32 },
	{ "R16Unorm", VkFormat::VK_FORMAT_R16_UNORM },
	{ "R16Snorm", VkFormat::VK_FORMAT_R16_SNORM },
	{ "R16Uscaled", VkFormat::VK_FORMAT_R16_USCALED },
	{ "R16Sscaled", VkFormat::VK_FORMAT_R16_SSCALED },
	{ "R16Uint", VkFormat::VK_FORMAT_R16_UINT },
	{ "R16Sint", VkFormat::VK_FORMAT_R16_SINT },
	{ "R16Sfloat", VkFormat::VK_FORMAT_R16_SFLOAT },
	{ "R16G16Unorm", VkFormat::VK_FORMAT_R16G16_UNORM },
	{ "R16G16Snorm", VkFormat::VK_FORMAT_R16G16_SNORM },
	{ "R16G16Uscaled", VkFormat::VK_FORMAT_R16G16_USCALED },
	{ "R16G16Sscaled", VkFormat::VK_FORMAT_R16G16_SSCALED },
	{ "R16G16Uint", VkFormat::VK_FORMAT_R16G16_UINT },
	{ "R16G16Sint", VkFormat::VK_FORMAT_R16G16_SINT },
	{ "R16G16Sfloat", VkFormat::VK_FORMAT_R16G16_SFLOAT },
	{ "R16G16B16Unorm", VkFormat::VK_FORMAT_R16G16B16_UNORM },
	{ "R16G16B16Snorm", VkFormat::VK_FORMAT_R16G16B16_SNORM },
	{ "R16G16B16Uscaled", VkFormat::VK_FORMAT_R16G16B16_USCALED },
	{ "R16G16B16Sscaled", VkFormat::VK_FORMAT_R16G16B16_SSCALED},
	{ "R16G16B16Uint", VkFormat::VK_FORMAT_R16G16B16_UINT },
	{ "R16G16B16Sint", VkFormat::VK_FORMAT_R16G16B16_SINT },
	{ "R16G16B16Sfloat", VkFormat::VK_FORMAT_R16G16B16_SFLOAT },
	{ "R16G16B16A16Unorm", VkFormat::VK_FORMAT_R16G16B16A16_UNORM },
	{ "R16G16B16A16Snorm", VkFormat::VK_FORMAT_R16G16B16A16_SNORM },
	{ "R16G16B16A16Uscaled", VkFormat::VK_FORMAT_R16G16B16A16_USCALED },
	{ "R16G16B16A16Sscaled", VkFormat::VK_FORMAT_R16G16B16A16_SSCALED },
	{ "R16G16B16A16Uint", VkFormat::VK_FORMAT_R16G16B16A16_UINT },
	{ "R16G16B16A16Sint", VkFormat::VK_FORMAT_R16G16B16A16_SINT },
	{ "R16G16B16A16Sfloat", VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT },
	{ "R32Uint", VkFormat::VK_FORMAT_R32_UINT },
	{ "R32Sint", VkFormat::VK_FORMAT_R32_SINT },
	{ "R32Sfloat", VkFormat::VK_FORMAT_R32_SFLOAT },
	{ "R32G32Uint", VkFormat::VK_FORMAT_R32G32_UINT },
	{ "R32G32Sint", VkFormat::VK_FORMAT_R32G32_SINT },
	{ "R32G32Sfloat", VkFormat::VK_FORMAT_R32G32_SFLOAT },
	{ "R32G32B32Uint", VkFormat::VK_FORMAT_R32G32B32_UINT },
	{ "R32G32B32Sint", VkFormat::VK_FORMAT_R32G32B32_SINT },
	{ "R32G32B32Sfloat", VkFormat::VK_FORMAT_R32G32B32_SFLOAT },
	{ "R32G32B32A32Uint", VkFormat::VK_FORMAT_R32G32B32A32_UINT },
	{ "R32G32B32A32Sint", VkFormat::VK_FORMAT_R32G32B32A32_SINT },
	{ "R32G32B32A32Sfloat", VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT },
	{ "R64Uint", VkFormat::VK_FORMAT_R64_UINT },
	{ "R64Sint", VkFormat::VK_FORMAT_R64_SINT },
	{ "R64Sfloat", VkFormat::VK_FORMAT_R64_SFLOAT },
	{ "R64G64Uint", VkFormat::VK_FORMAT_R64G64_UINT },
	{ "R64G64Sint", VkFormat::VK_FORMAT_R64G64_SINT },
	{ "R64G64Sfloat", VkFormat::VK_FORMAT_R64G64_SFLOAT },
	{ "R64G64B64Uint", VkFormat::VK_FORMAT_R64G64B64_UINT },
	{ "R64G64B64Sint", VkFormat::VK_FORMAT_R64G64B64_SINT },
	{ "R64G64B64Sfloat", VkFormat::VK_FORMAT_R64G64B64_SFLOAT },
	{ "R64G64B64A64Uint", VkFormat::VK_FORMAT_R64G64B64A64_UINT },
	{ "R64G64B64A64Sint", VkFormat::VK_FORMAT_R64G64B64A64_SINT },
	{ "R64G64B64A64Sfloat", VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT },
	{ "B10G11R11UfloatPack32", VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32 },
	{ "E5B9G9R9UfloatPack32", VkFormat::VK_FORMAT_E5B9G9R9_UFLOAT_PACK32 },
	{ "D16Unorm", VkFormat::VK_FORMAT_D16_UNORM },
	{ "X8D24UnormPack32", VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32 },
	{ "D32Sfloat", VkFormat::VK_FORMAT_D32_SFLOAT },
	{ "S8Uint", VkFormat::VK_FORMAT_S8_UINT },
	{ "D16UnormS8Uint", VkFormat::VK_FORMAT_D16_UNORM_S8_UINT },
	{ "D24UnormS8Uint", VkFormat::VK_FORMAT_D24_UNORM_S8_UINT },
	{ "D32SfloatS8Uint", VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT },
	{ "Bc1RgbUnormBlock", VkFormat::VK_FORMAT_BC1_RGB_UNORM_BLOCK },
	{ "Bc1RgbSrgbBlock", VkFormat::VK_FORMAT_BC1_RGB_SRGB_BLOCK },
	{ "Bc1RgbaUnormBlock", VkFormat::VK_FORMAT_BC1_RGBA_UNORM_BLOCK },
	{ "Bc1RgbaSrgbBlock", VkFormat::VK_FORMAT_BC1_RGBA_SRGB_BLOCK },
	{ "Bc2UnormBlock", VkFormat::VK_FORMAT_BC2_UNORM_BLOCK },
	{ "Bc2SrgbBlock", VkFormat::VK_FORMAT_BC2_SRGB_BLOCK },
	{ "Bc3UnormBlock", VkFormat::VK_FORMAT_BC3_UNORM_BLOCK },
	{ "Bc3SrgbBlock", VkFormat::VK_FORMAT_BC3_SRGB_BLOCK },
	{ "Bc4UnormBlock", VkFormat::VK_FORMAT_BC4_UNORM_BLOCK },
	{ "Bc4SnormBlock", VkFormat::VK_FORMAT_BC4_SNORM_BLOCK },
	{ "Bc5UnormBlock", VkFormat::VK_FORMAT_BC5_UNORM_BLOCK },
	{ "Bc5SnormBlock", VkFormat::VK_FORMAT_BC5_SNORM_BLOCK },
	{ "Bc6HUfloatBlock", VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK },
	{ "Bc6HSfloatBlock", VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK },
	{ "Bc7UnormBlock", VkFormat::VK_FORMAT_BC7_UNORM_BLOCK },
	{ "Bc7SrgbBlock", VkFormat::VK_FORMAT_BC7_SRGB_BLOCK }
};
#pragma endregion

static VkPipelineDepthStencilStateCreateInfo GetDefaultDepthStencilstate() {
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.pNext = nullptr;
	depthStencilCreateInfo.back.passOp = VkStencilOp::VK_STENCIL_OP_ZERO;
	depthStencilCreateInfo.depthBoundsTestEnable = false;
	depthStencilCreateInfo.depthCompareOp = VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCreateInfo.depthTestEnable = true;
	depthStencilCreateInfo.depthWriteEnable = true;
	depthStencilCreateInfo.front.passOp = VkStencilOp::VK_STENCIL_OP_KEEP;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	return depthStencilCreateInfo;
}

static VkPipelineRasterizationStateCreateInfo GetDefaultRasterState() {
	VkPipelineRasterizationStateCreateInfo rasterState = {};
	rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterState.pNext = nullptr;
	rasterState.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
	rasterState.frontFace = VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
	rasterState.depthClampEnable = false;
	rasterState.rasterizerDiscardEnable = false;
	rasterState.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL;
	rasterState.depthBiasEnable = false;
	rasterState.depthBiasConstantFactor = 0.0f;
	rasterState.depthBiasClamp = 0.0f;
	rasterState.depthBiasSlopeFactor = 0.0f;
	rasterState.lineWidth = 1.0f;
	return rasterState;
}

static VkPipelineColorBlendStateCreateInfo GetDefaultColorBlendState() {
	VkPipelineColorBlendStateCreateInfo blendStateInfo = {};
	blendStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendStateInfo.pNext = nullptr;
	blendStateInfo.blendConstants[0] = 0.0f;
	blendStateInfo.blendConstants[1] = 0.0f;
	blendStateInfo.blendConstants[2] = 0.0f;
	blendStateInfo.blendConstants[3] = 0.0f;
	blendStateInfo.logicOp = VkLogicOp::VK_LOGIC_OP_COPY;
	blendStateInfo.logicOpEnable = false;
	return blendStateInfo;
}

static VkPipelineColorBlendAttachmentState GetDefaultColorBlendAttachmentState() {
	VkPipelineColorBlendAttachmentState blendAttachStateInfo;
	blendAttachStateInfo.blendEnable = VK_FALSE;
	blendAttachStateInfo.srcColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	blendAttachStateInfo.dstColorBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	blendAttachStateInfo.colorBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
	blendAttachStateInfo.srcAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	blendAttachStateInfo.dstAlphaBlendFactor = VkBlendFactor::VK_BLEND_FACTOR_ZERO;
	blendAttachStateInfo.alphaBlendOp = VkBlendOp::VK_BLEND_OP_ADD;
	blendAttachStateInfo.colorWriteMask = VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT | VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT;
	return blendAttachStateInfo;
}

static VkPipelineInputAssemblyStateCreateInfo GetDefaultInputAssemblyState() {
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
	inputAssemblyState.primitiveRestartEnable = false;
	inputAssemblyState.pNext = nullptr;
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	return inputAssemblyState;
}
}