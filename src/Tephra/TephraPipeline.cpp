#include "TephraPipeline.h"
#include <fstream>
#include <sstream>
#include <json.hpp>
#include "TephraShader.h"
using nlohmann::json;
using Tephra::Pipeline;

enum SHADER_TYPES {
	VERTEX_SHADER,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	EVALUATION_SHADER,
	CONTROL_SHADER,
	COMPUTE_SHADER
};

#pragma region mappings
//static mappings to vulkan enums
std::unordered_map<std::string, vk::DescriptorType> Pipeline::ToDescriptorType = {
	{ "Sampler", vk::DescriptorType::eSampler},
	{ "CombinedImageSampler", vk::DescriptorType::eCombinedImageSampler },
	{ "SampledImage", vk::DescriptorType::eSampledImage },
	{ "StorageImage", vk::DescriptorType::eStorageImage },
	{ "UniformTexelBuffer", vk::DescriptorType::eUniformTexelBuffer },
	{ "StorageTexelBuffer", vk::DescriptorType::eStorageTexelBuffer },
	{ "UniformBuffer", vk::DescriptorType::eUniformBuffer },
	{ "StorageBuffer", vk::DescriptorType::eStorageBuffer },
	{ "UniformBufferDynamic", vk::DescriptorType::eUniformBufferDynamic },
	{ "StorageBufferDynamic", vk::DescriptorType::eStorageBufferDynamic },
	{ "InputAttachment", vk::DescriptorType::eInputAttachment }
};

std::unordered_map<std::string, vk::BlendFactor> Pipeline::ToBlendFactor = {
	{ "Zero", vk::BlendFactor::eZero },
	{ "One", vk::BlendFactor::eOne },
	{ "SrcColor", vk::BlendFactor::eSrcColor },
	{ "OneMinusSrcColor", vk::BlendFactor::eOneMinusSrcColor },
	{ "DstColor", vk::BlendFactor::eDstColor },
	{ "OneMinusDstColor", vk::BlendFactor::eOneMinusDstColor },
	{ "OneMinusSrcAlpha", vk::BlendFactor::eOneMinusSrcAlpha },
	{ "DstAlpha", vk::BlendFactor::eDstAlpha },
	{ "OneMinusDstAlpha", vk::BlendFactor::eOneMinusDstAlpha },
	{ "ConstantColor", vk::BlendFactor::eConstantColor },
	{ "OneMinusConstantColor", vk::BlendFactor::eOneMinusConstantColor },
	{ "ConstantAlpha", vk::BlendFactor::eConstantAlpha },
	{ "OneMinusConstantAlpha", vk::BlendFactor::eOneMinusConstantAlpha },
	{ "SrcAlphaSaturate", vk::BlendFactor::eSrcAlphaSaturate },
	{ "OneMinusConstantAlpha", vk::BlendFactor::eOneMinusConstantAlpha },
	{ "Src1Color", vk::BlendFactor::eSrc1Color },
	{ "OneMinusSrc1Color", vk::BlendFactor::eOneMinusSrc1Color },
	{ "Src1Alpha", vk::BlendFactor::eSrc1Alpha },
	{ "OneMinusSrc1Alpha", vk::BlendFactor::eOneMinusSrc1Alpha },
};

std::unordered_map<std::string, vk::BlendOp> Pipeline::ToBlendOp = {
	{ "Add", vk::BlendOp::eAdd },
	{ "Subtract", vk::BlendOp::eSubtract },
	{ "ReverseSubtract", vk::BlendOp::eReverseSubtract },
	{ "Min", vk::BlendOp::eMin },
	{ "Max", vk::BlendOp::eMax }
};

std::unordered_map<int, vk::ShaderStageFlagBits> Pipeline::ToShaderStage = {
	{ 0, vk::ShaderStageFlagBits::eVertex },
	{ 1, vk::ShaderStageFlagBits::eFragment },
	{ 2, vk::ShaderStageFlagBits::eGeometry },
	{ 3, vk::ShaderStageFlagBits::eTessellationEvaluation },
	{ 4, vk::ShaderStageFlagBits::eTessellationControl },
	{ 5, vk::ShaderStageFlagBits::eCompute }
};

std::unordered_map<std::string, vk::LogicOp> Pipeline::ToLogicOp = {
	{ "Clear", vk::LogicOp::eClear },
	{ "And", vk::LogicOp::eAnd },
	{ "AndReverse", vk::LogicOp::eAndReverse },
	{ "Copy", vk::LogicOp::eCopy },
	{ "AndInverted", vk::LogicOp::eAndInverted },
	{ "NoOp", vk::LogicOp::eNoOp },
	{ "Xor", vk::LogicOp::eXor },
	{ "Or", vk::LogicOp::eOr },
	{ "Nor", vk::LogicOp::eNor },
	{ "Equivalent", vk::LogicOp::eEquivalent },
	{ "Invert", vk::LogicOp::eInvert },
	{ "OrReverse", vk::LogicOp::eOrReverse },
	{ "CopyInverted", vk::LogicOp::eCopyInverted },
	{ "OrInverted", vk::LogicOp::eOrInverted },
	{ "Nand", vk::LogicOp::eNand },
	{ "Set", vk::LogicOp::eSet }
};

std::unordered_map<std::string, vk::StencilOp> Pipeline::ToStencilOp = {
	{ "Keep", vk::StencilOp::eKeep },
	{ "Zero", vk::StencilOp::eZero },
	{ "Replace", vk::StencilOp::eReplace },
	{ "IncrementAndClamp", vk::StencilOp::eIncrementAndClamp },
	{ "DecrementAndClamp", vk::StencilOp::eDecrementAndClamp },
	{ "Invert", vk::StencilOp::eInvert },
	{ "IncrementAndWrap", vk::StencilOp::eIncrementAndWrap },
	{ "DecrementAndWrap", vk::StencilOp::eDecrementAndWrap }
};

std::unordered_map<std::string, vk::CompareOp> Pipeline::ToCompareOp = {
	{ "Never", vk::CompareOp::eNever },
	{ "Less", vk::CompareOp::eLess },
	{ "Equal", vk::CompareOp::eEqual },
	{ "LessOrEqual", vk::CompareOp::eLessOrEqual },
	{ "Greater", vk::CompareOp::eGreater },
	{ "NotEqual", vk::CompareOp::eNotEqual },
	{ "GreaterOrEqual", vk::CompareOp::eGreaterOrEqual },
	{ "Always", vk::CompareOp::eAlways }
};

std::unordered_map<std::string, vk::PrimitiveTopology> Pipeline::ToPrimitiveTopology = {
	{ "PointList", vk::PrimitiveTopology::ePointList },
	{ "LineList", vk::PrimitiveTopology::eLineList },
	{ "LineStrip", vk::PrimitiveTopology::eLineStrip },
	{ "TriangleList", vk::PrimitiveTopology::eTriangleList },
	{ "TriangleStrip", vk::PrimitiveTopology::eTriangleStrip },
	{ "TriangleFan", vk::PrimitiveTopology::eTriangleFan },
	{ "LineListWithAdjacency", vk::PrimitiveTopology::eLineListWithAdjacency },
	{ "LineStripWithAdjacency", vk::PrimitiveTopology::eLineStripWithAdjacency },
	{ "TriangleListWithAdjacency", vk::PrimitiveTopology::eTriangleListWithAdjacency },
	{ "TriangleStripWithAdjacency", vk::PrimitiveTopology::eTriangleStripWithAdjacency },
	{ "PatchList", vk::PrimitiveTopology::ePatchList }
};


std::unordered_map<int, vk::SampleCountFlagBits> Pipeline::ToSampleCount = {
	{ 1, vk::SampleCountFlagBits::e1 },
	{ 2, vk::SampleCountFlagBits::e2 },
	{ 4, vk::SampleCountFlagBits::e4 },
	{ 8, vk::SampleCountFlagBits::e8 },
	{ 16, vk::SampleCountFlagBits::e16 },
	{ 32, vk::SampleCountFlagBits::e32 },
	{ 64, vk::SampleCountFlagBits::e64 }
};

std::unordered_map<std::string, vk::CullModeFlagBits> Pipeline::ToCullMode = {
	{ "None", vk::CullModeFlagBits::eNone },
	{ "Front", vk::CullModeFlagBits::eFront },
	{ "Back", vk::CullModeFlagBits::eBack },
	{ "FrontAndBack", vk::CullModeFlagBits::eFrontAndBack }
};

std::unordered_map<std::string, vk::FrontFace> Pipeline::ToFrontFace = {
	{ "CounterClockwise", vk::FrontFace::eCounterClockwise },
	{ "Clockwise", vk::FrontFace::eClockwise }
};

std::unordered_map<std::string, vk::PolygonMode> Pipeline::ToPolygonMode = {
	{ "Fill", vk::PolygonMode::eFill },
	{ "Line", vk::PolygonMode::eLine },
	{ "Point", vk::PolygonMode::ePoint }
};

std::unordered_map<std::string, vk::VertexInputRate> Pipeline::ToVertexInputRate = {
	{ "Instance", vk::VertexInputRate::eInstance },
	{ "Vertex", vk::VertexInputRate::eVertex }
};

std::unordered_map<std::string, vk::Format> Pipeline::ToFormat = {
	{ "Undefined", vk::Format::eUndefined },
	{ "R4G4UnormPack8", vk::Format::eR4G4UnormPack8 },
	{ "R4G4B4A4UnormPack16", vk::Format::eR4G4B4A4UnormPack16 },
	{ "B4G4R4A4UnormPack16", vk::Format::eB4G4R4A4UnormPack16 },
	{ "R5G6B5UnormPack16", vk::Format::eR5G6B5UnormPack16 },
	{ "B5G6R5UnormPack16", vk::Format::eB5G6R5UnormPack16 },
	{ "R5G5B5A1UnormPack16", vk::Format::eR5G5B5A1UnormPack16 },
	{ "B5G5R5A1UnormPack16", vk::Format::eB5G5R5A1UnormPack16 },
	{ "A1R5G5B5UnormPack16", vk::Format::eA1R5G5B5UnormPack16 },
	{ "R8Unorm", vk::Format::eR8Unorm },
	{ "R8Snorm", vk::Format::eR8Snorm },
	{ "R8Uscaled", vk::Format::eR8Uscaled },
	{ "R8Sscaled", vk::Format::eR8Sscaled },
	{ "R8Uint", vk::Format::eR8Uint },
	{ "R8Sint", vk::Format::eR8Sint },
	{ "R8Srgb", vk::Format::eR8Srgb },
	{ "R8G8Unorm", vk::Format::eR8G8Unorm },
	{ "R8G8Snorm", vk::Format::eR8G8Snorm },
	{ "R8G8Uscaled", vk::Format::eR8G8Uscaled },
	{ "R8G8Sscaled", vk::Format::eR8G8Sscaled },
	{ "R8G8Uint", vk::Format::eR8G8Uint },
	{ "R8G8Sint", vk::Format::eR8G8Sint },
	{ "R8G8Srgb", vk::Format::eR8G8Srgb },
	{ "R8G8B8Unorm", vk::Format::eR8G8B8Unorm },
	{ "R8G8B8Snorm", vk::Format::eR8G8B8Snorm },
	{ "R8G8B8Uscaled", vk::Format::eR8G8B8Uscaled },
	{ "R8G8B8Sscaled", vk::Format::eR8G8B8Sscaled },
	{ "R8G8B8Uint", vk::Format::eR8G8B8Uint },
	{ "R8G8B8Sint", vk::Format::eR8G8B8Sint },
	{ "R8G8B8Srgb", vk::Format::eR8G8B8Srgb },
	{ "B8G8R8Unorm", vk::Format::eB8G8R8Unorm },
	{ "B8G8R8Snorm", vk::Format::eB8G8R8Snorm },
	{ "B8G8R8Uscaled", vk::Format::eB8G8R8Uscaled },
	{ "B8G8R8Sscaled", vk::Format::eB8G8R8Sscaled },
	{ "B8G8R8Uint", vk::Format::eB8G8R8Uint },
	{ "B8G8R8Sint", vk::Format::eB8G8R8Sint },
	{ "B8G8R8Srgb", vk::Format::eB8G8R8Srgb },
	{ "R8G8B8A8Unorm", vk::Format::eR8G8B8A8Unorm },
	{ "R8G8B8A8Snorm", vk::Format::eR8G8B8A8Snorm },
	{ "R8G8B8A8Uscaled", vk::Format::eR8G8B8A8Uscaled },
	{ "R8G8B8A8Sscaled", vk::Format::eR8G8B8A8Sscaled },
	{ "R8G8B8A8Uint", vk::Format::eR8G8B8A8Uint },
	{ "R8G8B8A8Sint", vk::Format::eR8G8B8A8Sint },
	{ "R8G8B8A8Srgb", vk::Format::eR8G8B8A8Srgb },
	{ "B8G8R8A8Unorm", vk::Format::eB8G8R8A8Unorm },
	{ "B8G8R8A8Snorm", vk::Format::eB8G8R8A8Snorm },
	{ "B8G8R8A8Uscaled", vk::Format::eB8G8R8A8Uscaled },
	{ "B8G8R8A8Sscaled", vk::Format::eB8G8R8A8Sscaled },
	{ "B8G8R8A8Uint", vk::Format::eB8G8R8A8Uint },
	{ "B8G8R8A8Sint", vk::Format::eB8G8R8A8Sint },
	{ "B8G8R8A8Srgb", vk::Format::eB8G8R8A8Srgb },
	{ "A8B8G8R8UnormPack32", vk::Format::eA8B8G8R8UnormPack32 },
	{ "A8B8G8R8SnormPack32", vk::Format::eA8B8G8R8SnormPack32 },
	{ "A8B8G8R8UscaledPack32", vk::Format::eA8B8G8R8UscaledPack32 },
	{ "A8B8G8R8SscaledPack32", vk::Format::eA8B8G8R8SscaledPack32 },
	{ "A8B8G8R8UintPack32", vk::Format::eA8B8G8R8UintPack32 },
	{ "A8B8G8R8SintPack32", vk::Format::eA8B8G8R8SintPack32 },
	{ "A8B8G8R8SrgbPack32", vk::Format::eA8B8G8R8SrgbPack32 },
	{ "A2R10G10B10UnormPack32", vk::Format::eA2R10G10B10UnormPack32 },
	{ "A2R10G10B10SnormPack32", vk::Format::eA2R10G10B10SnormPack32 },
	{ "A2R10G10B10UscaledPack32", vk::Format::eA2R10G10B10UscaledPack32 },
	{ "A2R10G10B10SscaledPack32", vk::Format::eA2R10G10B10SscaledPack32 },
	{ "A2R10G10B10UintPack32", vk::Format::eA2R10G10B10UintPack32 },
	{ "A2R10G10B10SintPack32", vk::Format::eA2R10G10B10SintPack32 },
	{ "A2B10G10R10UnormPack32", vk::Format::eA2B10G10R10UnormPack32 },
	{ "A2B10G10R10SnormPack32", vk::Format::eA2B10G10R10SnormPack32 },
	{ "A2B10G10R10UscaledPack32", vk::Format::eA2B10G10R10UscaledPack32 },
	{ "A2B10G10R10SscaledPack32", vk::Format::eA2B10G10R10SscaledPack32 },
	{ "A2B10G10R10UintPack32", vk::Format::eA2B10G10R10UintPack32 },
	{ "A2B10G10R10SintPack32", vk::Format::eA2B10G10R10SintPack32 },
	{ "R16Unorm", vk::Format::eR16Unorm },
	{ "R16Snorm", vk::Format::eR16Snorm },
	{ "R16Uscaled", vk::Format::eR16Uscaled },
	{ "R16Sscaled", vk::Format::eR16Sscaled },
	{ "R16Uint", vk::Format::eR16Uint },
	{ "R16Sint", vk::Format::eR16Sint },
	{ "R16Sfloat", vk::Format::eR16Sfloat },
	{ "R16G16Unorm", vk::Format::eR16G16Unorm },
	{ "R16G16Snorm", vk::Format::eR16G16Snorm },
	{ "R16G16Uscaled", vk::Format::eR16G16Uscaled },
	{ "R16G16Sscaled", vk::Format::eR16G16Sscaled },
	{ "R16G16Uint", vk::Format::eR16G16Uint },
	{ "R16G16Sint", vk::Format::eR16G16Sint },
	{ "R16G16Sfloat", vk::Format::eR16G16Sfloat },
	{ "R16G16B16Unorm", vk::Format::eR16G16B16Unorm },
	{ "R16G16B16Snorm", vk::Format::eR16G16B16Snorm },
	{ "R16G16B16Uscaled", vk::Format::eR16G16B16Uscaled },
	{ "R16G16B16Sscaled", vk::Format::eR16G16B16Sscaled },
	{ "R16G16B16Uint", vk::Format::eR16G16B16Uint },
	{ "R16G16B16Sint", vk::Format::eR16G16B16Sint },
	{ "R16G16B16Sfloat", vk::Format::eR16G16B16Sfloat },
	{ "R16G16B16A16Unorm", vk::Format::eR16G16B16A16Unorm },
	{ "R16G16B16A16Snorm", vk::Format::eR16G16B16A16Snorm },
	{ "R16G16B16A16Uscaled", vk::Format::eR16G16B16A16Uscaled },
	{ "R16G16B16A16Sscaled", vk::Format::eR16G16B16A16Sscaled },
	{ "R16G16B16A16Uint", vk::Format::eR16G16B16A16Uint },
	{ "R16G16B16A16Sint", vk::Format::eR16G16B16A16Sint },
	{ "R16G16B16A16Sfloat", vk::Format::eR16G16B16A16Sfloat },
	{ "R32Uint", vk::Format::eR32Uint },
	{ "R32Sint", vk::Format::eR32Sint },
	{ "R32Sfloat", vk::Format::eR32Sfloat },
	{ "R32G32Uint", vk::Format::eR32G32Uint },
	{ "R32G32Sint", vk::Format::eR32G32Sint },
	{ "R32G32Sfloat", vk::Format::eR32G32Sfloat },
	{ "R32G32B32Uint", vk::Format::eR32G32B32Uint },
	{ "R32G32B32Sint", vk::Format::eR32G32B32Sint },
	{ "R32G32B32Sfloat", vk::Format::eR32G32B32Sfloat },
	{ "R32G32B32A32Uint", vk::Format::eR32G32B32A32Uint },
	{ "R32G32B32A32Sint", vk::Format::eR32G32B32A32Sint },
	{ "R32G32B32A32Sfloat", vk::Format::eR32G32B32A32Sfloat },
	{ "R64Uint", vk::Format::eR64Uint },
	{ "R64Sint", vk::Format::eR64Sint },
	{ "R64Sfloat", vk::Format::eR64Sfloat },
	{ "R64G64Uint", vk::Format::eR64G64Uint },
	{ "R64G64Sint", vk::Format::eR64G64Sint },
	{ "R64G64Sfloat", vk::Format::eR64G64Sfloat },
	{ "R64G64B64Uint", vk::Format::eR64G64B64Uint },
	{ "R64G64B64Sint", vk::Format::eR64G64B64Sint },
	{ "R64G64B64Sfloat", vk::Format::eR64G64B64Sfloat },
	{ "R64G64B64A64Uint", vk::Format::eR64G64B64A64Uint },
	{ "R64G64B64A64Sint", vk::Format::eR64G64B64A64Sint },
	{ "R64G64B64A64Sfloat", vk::Format::eR64G64B64A64Sfloat },
	{ "B10G11R11UfloatPack32", vk::Format::eB10G11R11UfloatPack32 },
	{ "E5B9G9R9UfloatPack32", vk::Format::eE5B9G9R9UfloatPack32 },
	{ "D16Unorm", vk::Format::eD16Unorm },
	{ "X8D24UnormPack32", vk::Format::eX8D24UnormPack32 },
	{ "D32Sfloat", vk::Format::eD32Sfloat },
	{ "S8Uint", vk::Format::eS8Uint },
	{ "D16UnormS8Uint", vk::Format::eD16UnormS8Uint },
	{ "D24UnormS8Uint", vk::Format::eD24UnormS8Uint },
	{ "D32SfloatS8Uint", vk::Format::eD32SfloatS8Uint },
	{ "Bc1RgbUnormBlock", vk::Format::eBc1RgbUnormBlock },
	{ "Bc1RgbSrgbBlock", vk::Format::eBc1RgbSrgbBlock },
	{ "Bc1RgbaUnormBlock", vk::Format::eBc1RgbaUnormBlock },
	{ "Bc1RgbaSrgbBlock", vk::Format::eBc1RgbaSrgbBlock },
	{ "Bc2UnormBlock", vk::Format::eBc2UnormBlock },
	{ "Bc2SrgbBlock", vk::Format::eBc2SrgbBlock },
	{ "Bc3UnormBlock", vk::Format::eBc3UnormBlock },
	{ "Bc3SrgbBlock", vk::Format::eBc3SrgbBlock },
	{ "Bc4UnormBlock", vk::Format::eBc4UnormBlock },
	{ "Bc4SnormBlock", vk::Format::eBc4SnormBlock },
	{ "Bc5UnormBlock", vk::Format::eBc5UnormBlock },
	{ "Bc5SnormBlock", vk::Format::eBc5SnormBlock },
	{ "Bc6HUfloatBlock", vk::Format::eBc6HUfloatBlock },
	{ "Bc6HSfloatBlock", vk::Format::eBc6HSfloatBlock },
	{ "Bc7UnormBlock", vk::Format::eBc7UnormBlock },
	{ "Bc7SrgbBlock", vk::Format::eBc7SrgbBlock },
	{ "Etc2R8G8B8UnormBlock", vk::Format::eEtc2R8G8B8UnormBlock },
	{ "Etc2R8G8B8SrgbBlock", vk::Format::eEtc2R8G8B8SrgbBlock },
	{ "Etc2R8G8B8A1UnormBlock", vk::Format::eEtc2R8G8B8A1UnormBlock },
	{ "Etc2R8G8B8A1SrgbBlock", vk::Format::eEtc2R8G8B8A1SrgbBlock },
	{ "Etc2R8G8B8A8UnormBlock", vk::Format::eEtc2R8G8B8A8UnormBlock },
	{ "Etc2R8G8B8A8SrgbBlock", vk::Format::eEtc2R8G8B8A8SrgbBlock },
	{ "EacR11UnormBlock", vk::Format::eEacR11UnormBlock },
	{ "EacR11SnormBlock", vk::Format::eEacR11SnormBlock },
	{ "EacR11G11UnormBlock", vk::Format::eEacR11G11UnormBlock },
	{ "EacR11G11SnormBlock", vk::Format::eEacR11G11SnormBlock },
	{ "Astc4x4UnormBlock", vk::Format::eAstc4x4UnormBlock },
	{ "Astc4x4SrgbBlock", vk::Format::eAstc4x4SrgbBlock },
	{ "Astc5x4UnormBlock", vk::Format::eAstc5x4UnormBlock },
	{ "Astc5x4UnormBlock", vk::Format::eAstc5x4UnormBlock },
	{ "Astc5x4SrgbBlock", vk::Format::eAstc5x4SrgbBlock },
	{ "Astc5x5UnormBlock", vk::Format::eAstc5x5UnormBlock },
	{ "Astc5x5SrgbBlock", vk::Format::eAstc5x5SrgbBlock },
	{ "Astc6x5UnormBlock", vk::Format::eAstc6x5UnormBlock },
	{ "Astc6x5SrgbBlock", vk::Format::eAstc6x5SrgbBlock },
	{ "Astc6x6UnormBlock", vk::Format::eAstc6x6UnormBlock },
	{ "Astc6x6SrgbBlock", vk::Format::eAstc6x6SrgbBlock },
	{ "Astc8x5UnormBlock", vk::Format::eAstc8x5UnormBlock },
	{ "Astc8x5SrgbBlock", vk::Format::eAstc8x5SrgbBlock },
	{ "Astc8x6UnormBlock", vk::Format::eAstc8x6UnormBlock },
	{ "Astc8x6SrgbBlock", vk::Format::eAstc8x6SrgbBlock },
	{ "Astc8x8UnormBlock", vk::Format::eAstc8x8UnormBlock },
	{ "Astc8x8SrgbBlock", vk::Format::eAstc8x8SrgbBlock },
	{ "Astc10x5UnormBlock", vk::Format::eAstc10x5UnormBlock },
	{ "Astc10x5SrgbBlock", vk::Format::eAstc10x5SrgbBlock },
	{ "Astc10x6UnormBlock", vk::Format::eAstc10x6UnormBlock },
	{ "Astc10x6SrgbBlock", vk::Format::eAstc10x6SrgbBlock },
	{ "Astc10x8UnormBlock", vk::Format::eAstc10x8UnormBlock },
	{ "Astc10x8SrgbBlock", vk::Format::eAstc10x8SrgbBlock },
	{ "Astc10x10UnormBlock", vk::Format::eAstc10x10UnormBlock },
	{ "Astc10x10SrgbBlock", vk::Format::eAstc10x10SrgbBlock },
	{ "Astc12x10UnormBlock", vk::Format::eAstc12x10UnormBlock },
	{ "Astc12x10SrgbBlock", vk::Format::eAstc12x10SrgbBlock },
	{ "Astc12x12UnormBlock", vk::Format::eAstc12x12UnormBlock },
	{ "Astc12x12SrgbBlock", vk::Format::eAstc12x12SrgbBlock },
	{ "Pvrtc12BppUnormBlockIMG", vk::Format::ePvrtc12BppUnormBlockIMG },
	{ "Pvrtc14BppUnormBlockIMG", vk::Format::ePvrtc14BppUnormBlockIMG },
	{ "Pvrtc22BppUnormBlockIMG", vk::Format::ePvrtc22BppUnormBlockIMG },
	{ "Pvrtc24BppUnormBlockIMG", vk::Format::ePvrtc24BppUnormBlockIMG },
	{ "Pvrtc12BppSrgbBlockIMG", vk::Format::ePvrtc12BppSrgbBlockIMG },
	{ "Pvrtc14BppSrgbBlockIMG", vk::Format::ePvrtc14BppSrgbBlockIMG },
	{ "Pvrtc22BppSrgbBlockIMG", vk::Format::ePvrtc22BppSrgbBlockIMG },
	{ "Pvrtc24BppSrgbBlockIMG", vk::Format::ePvrtc24BppSrgbBlockIMG }
};
#pragma endregion

Pipeline::Pipeline() {

}

Pipeline::~Pipeline() {

}

vk::PipelineColorBlendAttachmentState GetDefaultColorBlendAttachmentState() {
	vk::PipelineColorBlendAttachmentState blendAttachStateInfo;
	blendAttachStateInfo.blendEnable = VK_FALSE;
	blendAttachStateInfo.srcColorBlendFactor = vk::BlendFactor::eZero;
	blendAttachStateInfo.dstColorBlendFactor = vk::BlendFactor::eZero;
	blendAttachStateInfo.colorBlendOp = vk::BlendOp::eAdd;
	blendAttachStateInfo.srcAlphaBlendFactor = vk::BlendFactor::eZero;
	blendAttachStateInfo.dstAlphaBlendFactor = vk::BlendFactor::eZero;
	blendAttachStateInfo.alphaBlendOp = vk::BlendOp::eAdd;
	blendAttachStateInfo.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	return blendAttachStateInfo;
}

vk::PipelineColorBlendAttachmentState ReadColorBlendAttachmentState(const json& blendJson) {
	vk::PipelineColorBlendAttachmentState blendAttachStateInfo = GetDefaultColorBlendAttachmentState();
	if (blendJson.find("BlendEnable") != blendJson.end()) {
		blendAttachStateInfo.blendEnable = TO_VK_BOOL(blendJson["BlendEnable"]);
	}
	if (blendJson.find("SrcColorBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.srcColorBlendFactor = Pipeline::ToBlendFactor[blendJson["SrcColorBlendFactor"]];
	}
	if (blendJson.find("DstColorBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.dstColorBlendFactor = Pipeline::ToBlendFactor[blendJson["DstColorBlendFactor"]];
	}
	if (blendJson.find("ColorBlendOp") != blendJson.end()) {
		blendAttachStateInfo.colorBlendOp = Pipeline::ToBlendOp[blendJson["ColorBlendOp"]];
	}
	if (blendJson.find("SrcAlphaBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.srcAlphaBlendFactor = Pipeline::ToBlendFactor[blendJson["SrcAlphaBlendFactor"]];
	}
	if (blendJson.find("DstAlphaBlendFactor") != blendJson.end()) {
		blendAttachStateInfo.dstAlphaBlendFactor = Pipeline::ToBlendFactor[blendJson["DstAlphaBlendFactor"]];
	}
	if (blendJson.find("AlphaBlendOp") != blendJson.end()) {
		blendAttachStateInfo.alphaBlendOp = Pipeline::ToBlendOp[blendJson["AlphaBlendOp"]];
	}
	//add color write mask
	return blendAttachStateInfo;
}

vk::PipelineColorBlendStateCreateInfo GetDefaultColorBlendState() {
	vk::PipelineColorBlendStateCreateInfo blendStateInfo;
	blendStateInfo.blendConstants[0] = 0.0f;
	blendStateInfo.blendConstants[1] = 0.0f;
	blendStateInfo.blendConstants[2] = 0.0f;
	blendStateInfo.blendConstants[3] = 0.0f;
	blendStateInfo.logicOp = vk::LogicOp::eCopy;
	blendStateInfo.logicOpEnable = false;
	return blendStateInfo;
}

vk::PipelineColorBlendStateCreateInfo ReadColorBlendState(const json& blendState) {
	vk::PipelineColorBlendStateCreateInfo blendStateInfo = GetDefaultColorBlendState();
	if (blendState.find("LogicOp") != blendState.end()) {
		blendStateInfo.logicOp = Pipeline::ToLogicOp[blendState["LogicOp"]];
	}
	if (blendState.find("logicOpEnable") != blendState.end()) {
		blendStateInfo.logicOpEnable = TO_VK_BOOL(blendState["logicOpEnable"]);
	}
	if (blendState.find("BlendConstants") != blendState.end()) {
		blendStateInfo.blendConstants[0] = blendState["BlendConstants"][0];
		blendStateInfo.blendConstants[1] = blendState["BlendConstants"][1];
		blendStateInfo.blendConstants[2] = blendState["BlendConstants"][2];
		blendStateInfo.blendConstants[3] = blendState["BlendConstants"][3];
	}
	return blendStateInfo;
}

vk::PipelineDepthStencilStateCreateInfo GetDefaultDepthStencilstate() {
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo;
	depthStencilCreateInfo.back = vk::StencilOp::eZero;
	depthStencilCreateInfo.depthBoundsTestEnable = true;
	depthStencilCreateInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;
	depthStencilCreateInfo.depthTestEnable = true;
	depthStencilCreateInfo.depthWriteEnable = true;
	depthStencilCreateInfo.front = vk::StencilOp::eKeep;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	return depthStencilCreateInfo;
}

vk::PipelineDepthStencilStateCreateInfo ReadDepthStencilstate(const json& depthState) {
	vk::PipelineDepthStencilStateCreateInfo depthStencilCreateInfo = GetDefaultDepthStencilstate();
	if (depthState.find("Back") != depthState.end()) {
		depthStencilCreateInfo.back = Pipeline::ToStencilOp[depthState["Back"]];
	}
	if (depthState.find("Front") != depthState.end()) {
		depthStencilCreateInfo.front = Pipeline::ToStencilOp[depthState["Front"]];
	}
	if (depthState.find("DepthBoundsTestEnable") != depthState.end()) {
		depthStencilCreateInfo.depthBoundsTestEnable = TO_VK_BOOL(depthState["DepthBoundsTestEnable"]);
	}
	if (depthState.find("DepthCompareOp") != depthState.end()) {
		depthStencilCreateInfo.depthCompareOp = Pipeline::ToCompareOp[depthState["DepthCompareOp"]];
	}
	if (depthState.find("DepthTestEnable") != depthState.end()) {
		depthStencilCreateInfo.depthTestEnable = TO_VK_BOOL(depthState["DepthTestEnable"]);
	}
	if (depthState.find("DepthWriteEnable") != depthState.end()) {
		depthStencilCreateInfo.depthWriteEnable = TO_VK_BOOL(depthState["DepthWriteEnable"]);
	}
	if (depthState.find("MaxDepthBounds") != depthState.end()) {
		depthStencilCreateInfo.maxDepthBounds = depthState["MaxDepthBounds"];
	}
	if (depthState.find("MinDepthBounds") != depthState.end()) {
		depthStencilCreateInfo.minDepthBounds = depthState["MinDepthBounds"];
	}
	return depthStencilCreateInfo;
}

vk::PipelineMultisampleStateCreateInfo GetDefaultMultiSampleState() {
	vk::PipelineMultisampleStateCreateInfo msState;
	msState.rasterizationSamples = vk::SampleCountFlagBits::e1;
	msState.alphaToCoverageEnable = false;
	msState.alphaToOneEnable = false;
	msState.minSampleShading = 0.0f;
	msState.sampleShadingEnable = false;
	msState.pSampleMask = nullptr;
	return msState;
}

vk::PipelineMultisampleStateCreateInfo ReadMultiSampleState(const json& msStateJson) {
	vk::PipelineMultisampleStateCreateInfo msState = GetDefaultMultiSampleState();
	if (msStateJson.find("RasterizationSamples") != msStateJson.end()) {
		msState.rasterizationSamples = Pipeline::ToSampleCount[msStateJson["RasterizationSamples"]];
	}
	if (msStateJson.find("SampleShadingEnable") != msStateJson.end()) {
		msState.sampleShadingEnable = TO_VK_BOOL(msStateJson["SampleShadingEnable"]);
	}
	if (msStateJson.find("MinSampleShading") != msStateJson.end()) {
		msState.minSampleShading = msStateJson["MinSampleShading"];
	}
	if (msStateJson.find("AlphaToCoverageEnable") != msStateJson.end()) {
		msState.alphaToCoverageEnable = TO_VK_BOOL(msStateJson["AlphaToCoverageEnable"]);
	}
	if (msStateJson.find("AlphaToOneEnable") != msStateJson.end()) {
		msState.alphaToOneEnable = TO_VK_BOOL(msStateJson["AlphaToOneEnable"]);
	}
	return msState;
}

vk::PipelineRasterizationStateCreateInfo GetDefaultRasterState() {
	vk::PipelineRasterizationStateCreateInfo rasterState;
	rasterState.cullMode = vk::CullModeFlagBits::eBack;
	rasterState.frontFace = vk::FrontFace::eCounterClockwise;
	rasterState.depthClampEnable = false;
	rasterState.rasterizerDiscardEnable = false;
	rasterState.polygonMode = vk::PolygonMode::eFill;
	rasterState.depthBiasEnable = false;
	rasterState.depthBiasConstantFactor = 0.0f;
	rasterState.depthBiasClamp = 0.0f;
	rasterState.depthBiasSlopeFactor = 0.0f;
	rasterState.lineWidth = 1.0f;
	return rasterState;
}

vk::PipelineRasterizationStateCreateInfo ReadRasterState(const json& rasterStateJson) {
	vk::PipelineRasterizationStateCreateInfo rState = GetDefaultRasterState();
	if (rasterStateJson.find("CullMode") != rasterStateJson.end()) {
		rState.cullMode = Pipeline::ToCullMode[rasterStateJson["CullMode"]];
	}
	if (rasterStateJson.find("FrontFace") != rasterStateJson.end()) {
		rState.frontFace = Pipeline::ToFrontFace[rasterStateJson["FrontFace"]];
	}
	if (rasterStateJson.find("PolygonMode") != rasterStateJson.end()) {
		rState.polygonMode = Pipeline::ToPolygonMode[rasterStateJson["PolygonMode"]];
	}
	if (rasterStateJson.find("DepthClampEnable") != rasterStateJson.end()) {
		rState.depthClampEnable = TO_VK_BOOL(rasterStateJson["DepthClampEnable"]);
	}
	if (rasterStateJson.find("RasterizerDiscardEnable") != rasterStateJson.end()) {
		rState.rasterizerDiscardEnable = TO_VK_BOOL(rasterStateJson["RasterizerDiscardEnable"]);
	}
	if (rasterStateJson.find("DepthBiasEnable") != rasterStateJson.end()) {
		rState.depthBiasEnable = TO_VK_BOOL(rasterStateJson["DepthBiasEnable"]);
	}
	if (rasterStateJson.find("DepthBiasConstantFactor") != rasterStateJson.end()) {
		rState.depthBiasConstantFactor = rasterStateJson["DepthBiasConstantFactor"];
	}
	if (rasterStateJson.find("DepthBiasClamp") != rasterStateJson.end()) {
		rState.depthBiasClamp = rasterStateJson["DepthBiasClamp"];
	}
	if (rasterStateJson.find("DepthBiasSlopeFactor") != rasterStateJson.end()) {
		rState.depthBiasSlopeFactor = rasterStateJson["DepthBiasSlopeFactor"];
	}
	if (rasterStateJson.find("LineWidth") != rasterStateJson.end()) {
		rState.lineWidth = rasterStateJson["LineWidth"];
	}
	return rState;
}

void Pipeline::LoadPipelineFromFile(const vk::Device& device, const std::string& filename, vk::Viewport vp, vk::RenderPass renderPass) {
	std::ifstream fin(filename);
	if (!fin.is_open()) {
		fin.close();
		return;
	}
	json root;
	try {
		fin >> root;
	}
	catch(std::exception e){
		printf("json: %s\n", e.what());
		return;
	}
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo;
	//Find shaders
	m_ShaderBits = 0;
	if (root.find("Shaders") != root.end()) {
		json shaders = root["Shaders"];
		if (shaders.find("Vertex") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Vertex"]));
			m_ShaderBits |= 1 << VERTEX_SHADER;
		}
		if (shaders.find("Fragment") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Fragment"]));
			m_ShaderBits |= 1 << FRAGMENT_SHADER;
		}
		if (shaders.find("Geometry") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Geometry"]));
			m_ShaderBits |= 1 << GEOMETRY_SHADER;
		}
		if (shaders.find("Evaluation") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Evaluation"]));
			m_ShaderBits |= 1 << EVALUATION_SHADER;
		}
		if (shaders.find("Control") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Control"]));
			m_ShaderBits |= 1 << CONTROL_SHADER;
		}
		if (shaders.find("Compute") != shaders.end()) {
			m_Shaders.push_back(LoadShader(device, shaders["Compute"]));
			m_ShaderBits |= 1 << COMPUTE_SHADER;
		}
	}
	else {
		return;
	}
	//descriptor set
	if (root.find("DescriptorSetLayout") != root.end()) {
		json descSetLayout = root["DescriptorSetLayout"];
		std::vector<vk::DescriptorSetLayoutBinding> bindings;

		for (auto& bind : descSetLayout) {
			vk::DescriptorSetLayoutBinding binding;
			binding.binding = bind["Binding"];
			binding.descriptorCount = bind["Count"];
			binding.descriptorType = ToDescriptorType[bind["Type"]];
			binding.stageFlags = vk::ShaderStageFlagBits::eAll; //keep the stage set to all for now
			bindings.push_back(binding);
		}

		vk::DescriptorSetLayoutCreateInfo descSetCreateInfo;
		descSetCreateInfo.bindingCount = bindings.size();
		descSetCreateInfo.pBindings = bindings.data();
		vk::DescriptorSetLayout layout;
		layout = device.createDescriptorSetLayout(descSetCreateInfo);
		m_DescSetLayouts.push_back(layout);
	}
	//push constants
	std::vector<vk::PushConstantRange> pushConstRanges;
	if (root.find("PushConstantRanges") != root.end()) {
		json pushConstants = root["PushConstantRanges"];
		for (auto& pc : pushConstants) {
			vk::PushConstantRange pushConst;
			pushConst.stageFlags = vk::ShaderStageFlagBits::eAll; //keep the stage set to all for now
			pushConst.size = pc["Size"];
			pushConst.offset = pc["Offset"];
			pushConstRanges.push_back(pushConst);
		}
	}
	//pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo;
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstRanges.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstRanges.size();
	pipelineLayoutCreateInfo.pSetLayouts = m_DescSetLayouts.data();
	pipelineLayoutCreateInfo.setLayoutCount = m_DescSetLayouts.size();
	m_PipelineLayout = device.createPipelineLayout(pipelineLayoutCreateInfo);

	//color blend state
	vk::PipelineColorBlendStateCreateInfo blendStateInfo;
	std::vector<vk::PipelineColorBlendAttachmentState> ColorblendAttachmentStates;
	if (root.find("ColorBlendState") != root.end()) {
		json ColorBlendStateJson = root["ColorBlendState"];
		blendStateInfo = ReadColorBlendState(ColorBlendStateJson);
		//blend attachments
		if (ColorBlendStateJson.find("ColorBlendAttachmentStates") != ColorBlendStateJson.end()) {
			json blendAttachmentStatesJson = ColorBlendStateJson["ColorBlendAttachmentStates"];
			for (auto& state : blendAttachmentStatesJson) {
				ColorblendAttachmentStates.push_back(ReadColorBlendAttachmentState(state));
			}
		}
		else {
			ColorblendAttachmentStates.push_back(GetDefaultColorBlendAttachmentState());
		}
	}
	else {
		blendStateInfo = GetDefaultColorBlendState();
	}
	blendStateInfo.attachmentCount = ColorblendAttachmentStates.size();
	blendStateInfo.pAttachments = ColorblendAttachmentStates.data();
	//depth stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencilState;
	if (root.find("DepthStencilState") != root.end()) {
		json depthStencilJson = root["DepthStencilState"];
		depthStencilState = ReadDepthStencilstate(depthStencilJson);
	}
	else {
		depthStencilState = GetDefaultDepthStencilstate();
	}
	//input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	if (root.find("InputAssemblyState") != root.end()) {
		json inputAssemJson = root["InputAssemblyState"];
		if(inputAssemJson.find("PrimitiveRestartEnable") != inputAssemJson.end()){
			inputAssemblyInfo.primitiveRestartEnable = TO_VK_BOOL(inputAssemJson["PrimitiveRestartEnable"]);
		}
		else {
			inputAssemblyInfo.primitiveRestartEnable = false;
		}
		if (inputAssemJson.find("Topology") != inputAssemJson.end()) {
			inputAssemblyInfo.topology = ToPrimitiveTopology[ inputAssemJson["Topology"]];
		}
		else {
			inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		}
	}
	else {
		inputAssemblyInfo.topology = vk::PrimitiveTopology::eTriangleList;
		inputAssemblyInfo.primitiveRestartEnable = false;
	}
	// ms state
	vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
	if (root.find("MultiSampleState") != root.end()) {
		json msStateJson = root["MultiSampleState"];
		multisampleStateInfo = ReadMultiSampleState(msStateJson);
	}
	else {
		multisampleStateInfo = GetDefaultMultiSampleState();
	}
	//raster state
	vk::PipelineRasterizationStateCreateInfo rasterInfo;
	if (root.find("RasterizationState") != root.end()) {
		json rsStateJson = root["RasterizationState"];
		rasterInfo = ReadRasterState(rsStateJson);
	}
	else {
		rasterInfo = GetDefaultRasterState();
	}
	//tesselation state
	vk::PipelineTessellationStateCreateInfo tesselationstate;
	if (root.find("TesselationState") != root.end()) {
		json tStateJson = root["TesselationState"];
		if (tStateJson.find("PatchControlPoints") != tStateJson.end()) {
			tesselationstate.patchControlPoints = tStateJson["PatchControlPoints"];
		}else {
			tesselationstate.patchControlPoints = 1;
		}
	}
	else {
		pipelineCreateInfo.pTessellationState = nullptr;
	}
	//shader stages
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
	for (int i = 0; i < 6; i++) {
		vk::PipelineShaderStageCreateInfo stage;
		if (m_ShaderBits & (1 << i)) {
			stage.module = m_Shaders[i];
			stage.stage = ToShaderStage[i];
			stage.pName = "main";
			stage.pSpecializationInfo = nullptr;
			shaderStages.push_back(stage);
		}
	}
	//vertex state
	//Vertex state must either be set in json or provided with a default before calling this function
	vk::PipelineVertexInputStateCreateInfo* vertexState = nullptr;
	std::vector<vk::VertexInputBindingDescription> vertexBindings;
	std::vector<vk::VertexInputAttributeDescription> vertexInputDescs;
	if (root.find("VertexInputState") != root.end()) {
		json vertexStateJson = root["VertexInputState"];
		//bindings
		if (vertexStateJson.find("InputBindings") != vertexStateJson.end()) {
			json bindings = vertexStateJson["InputBindings"];
			for (auto& bind : bindings) {
				vk::VertexInputBindingDescription bindDesc;
				bindDesc.binding = bind["Binding"];
				bindDesc.inputRate = ToVertexInputRate[bind["InputRate"]];
				bindDesc.stride = bind["Stride"];
				vertexBindings.push_back(bindDesc);
			}
		}
		//input attributes
		if (vertexStateJson.find("InputAttributes") != vertexStateJson.end()) {
			json attributes = vertexStateJson["InputAttributes"];
			for (auto& attr : attributes) {
				vk::VertexInputAttributeDescription attrDesc;
				attrDesc.binding = attr["Binding"];
				attrDesc.format = ToFormat[attr["Format"]];
				attrDesc.location = attr["Location"];
				attrDesc.offset = attr["Offset"];
				vertexInputDescs.push_back(attrDesc);
			}
		}
		vertexState = new vk::PipelineVertexInputStateCreateInfo();
		vertexState->pVertexBindingDescriptions = vertexBindings.data();
		vertexState->vertexBindingDescriptionCount = vertexBindings.size();
		vertexState->pVertexAttributeDescriptions = vertexInputDescs.data();
		vertexState->vertexAttributeDescriptionCount = vertexInputDescs.size();
	}


	//viewport state
	vk::PipelineViewportStateCreateInfo viewportState;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &vp;
	viewportState.scissorCount = 1;
	vk::Rect2D scissor;
	scissor.extent = vk::Extent2D(vp.width, vp.height);
	scissor.offset = vk::Offset2D(0, 0);
	viewportState.pScissors = &scissor;

	//put everything together
	pipelineCreateInfo.layout = m_PipelineLayout;
	pipelineCreateInfo.pColorBlendState = &blendStateInfo;
	
	pipelineCreateInfo.pDepthStencilState = nullptr; // &depthStencilState;
	pipelineCreateInfo.pDynamicState = nullptr;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineCreateInfo.pMultisampleState = &multisampleStateInfo;
	pipelineCreateInfo.pRasterizationState = &rasterInfo;
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pTessellationState = nullptr; // &tesselationstate;
	pipelineCreateInfo.pVertexInputState = (vertexState == nullptr) ? &m_DefaultVertexState : vertexState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.renderPass = renderPass;
	pipelineCreateInfo.stageCount = shaderStages.size();

	m_Pipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);

	//clean up
	if (vertexState) delete vertexState;
}

void Pipeline::SetDefaultVertexState(const vk::PipelineVertexInputStateCreateInfo& vertexState) {
	m_DefaultVertexState = vertexState;
	m_DefaultVertexStateSet = true;
}

std::vector<vk::DescriptorSetLayout>& Pipeline::GetDescriptorSetLayouts() {
	return m_DescSetLayouts;
}

vk::Pipeline Pipeline::GetPipeline() {
	return m_Pipeline;
}

vk::PipelineLayout Pipeline::GetPipelineLayout() {
	return m_PipelineLayout;
}