#pragma once
#include <glm/glm.hpp>
#include "volk.h"
namespace smug {
namespace Geometry {
struct Vertex {
	glm::vec3 PosL;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoord;
};

static eastl::vector<VkVertexInputAttributeDescription> VertexAttributes;
static eastl::vector<VkVertexInputBindingDescription> VertexBindings;
static VkPipelineVertexInputStateCreateInfo VertexState;

static VkPipelineVertexInputStateCreateInfo GetVertexState() {
	static bool first = true;
	if (first) {
		//bindings
		VkVertexInputBindingDescription bindDesc;
		bindDesc.binding = 0;
		bindDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		bindDesc.stride = sizeof(glm::vec3);
		VertexBindings.push_back(bindDesc);
		bindDesc.binding = 1;
		bindDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		bindDesc.stride = sizeof(glm::vec3);
		VertexBindings.push_back(bindDesc);
		bindDesc.binding = 2;
		bindDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		bindDesc.stride = sizeof(glm::vec3);
		VertexBindings.push_back(bindDesc);
		bindDesc.binding = 3;
		bindDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;
		bindDesc.stride = sizeof(glm::vec2);
		VertexBindings.push_back(bindDesc);
		//input attributes
		VkVertexInputAttributeDescription attrDesc;
		attrDesc.binding = 0;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.location = 0;
		attrDesc.offset = 0;
		VertexAttributes.push_back(attrDesc);//posl
		attrDesc.binding = 1;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.location = 1;
		attrDesc.offset = 0;
		VertexAttributes.push_back(attrDesc);//normal
		attrDesc.binding = 2;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		attrDesc.location = 2;
		attrDesc.offset = 0;
		VertexAttributes.push_back(attrDesc); //tangent
		attrDesc.binding = 3;
		attrDesc.format = VkFormat::VK_FORMAT_R32G32_SFLOAT;
		attrDesc.location = 3;
		attrDesc.offset = 0;
		VertexAttributes.push_back(attrDesc); //texcoord

		VertexState.pVertexBindingDescriptions = VertexBindings.data();
		VertexState.vertexBindingDescriptionCount = (uint32_t)VertexBindings.size();
		VertexState.pVertexAttributeDescriptions = VertexAttributes.data();
		VertexState.vertexAttributeDescriptionCount = (uint32_t)VertexAttributes.size();
		VertexState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexState.pNext = nullptr;
		first = false;
	}
	return VertexState;
}
}
}