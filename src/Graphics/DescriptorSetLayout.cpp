#include "DescriptorSetLayout.h"
#include <spirv_cross/spirv_glsl.hpp>

DescriptorSetLayout::DescriptorSetLayout() {

}

DescriptorSetLayout::~DescriptorSetLayout() {

}

//there are 4 DescriptorSets set up by the engine
//set 0 contains the perframe/viewport buffer
//set 1 contains lighting data
//set 2 contains the perObject buffer

//set 3 contains the material

void DescriptorSetLayout::InitFromSpirV(const uint32_t* buffer, uint32_t bufferLength) {
	spirv_cross::CompilerGLSL glslCompiler(buffer, bufferLength / sizeof(uint32_t));
	spirv_cross::ShaderResources resources = glslCompiler.get_shader_resources();

	vk::DescriptorSetLayoutCreateInfo descSetCreateInfo;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;

	vk::DescriptorSetLayoutBinding binding;
	binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
}