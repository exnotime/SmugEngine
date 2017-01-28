#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

namespace Vertex {
	struct Vertex {
		glm::vec3 PosL;
		glm::vec3 Normal;
	};

	static std::vector<vk::VertexInputAttributeDescription> VertexAttributes;
	static std::vector<vk::VertexInputBindingDescription> VertexBindings;
	static vk::PipelineVertexInputStateCreateInfo VertexState;

	static vk::PipelineVertexInputStateCreateInfo GetVertexState() {
		//bindings
		vk::VertexInputBindingDescription bindDesc;
		bindDesc.binding = 0;
		bindDesc.inputRate = vk::VertexInputRate::eVertex;
		bindDesc.stride = sizeof(Vertex);
		VertexBindings.push_back(bindDesc);
		//input attributes
		vk::VertexInputAttributeDescription attrDesc;
		attrDesc.binding = 0;
		attrDesc.format = vk::Format::eR32G32B32Sfloat;
		attrDesc.location = 0;
		attrDesc.offset = 0;
		VertexAttributes.push_back(attrDesc);//posl
		attrDesc.binding = 0;
		attrDesc.format = vk::Format::eR32G32B32Sfloat;
		attrDesc.location = 1;
		attrDesc.offset = sizeof(glm::vec3);
		VertexAttributes.push_back(attrDesc);//normal

		VertexState.pVertexBindingDescriptions = VertexBindings.data();
		VertexState.vertexBindingDescriptionCount = VertexBindings.size();
		VertexState.pVertexAttributeDescriptions = VertexAttributes.data();
		VertexState.vertexAttributeDescriptionCount = VertexAttributes.size();
		return VertexState;
	}
}