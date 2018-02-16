#pragma once
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
namespace smug {
	namespace Geometry {
		struct Vertex {
			glm::vec3 PosL;
			glm::vec3 Normal;
			glm::vec3 Tangent;
			glm::vec2 TexCoord;
		};

		static std::vector<vk::VertexInputAttributeDescription> VertexAttributes;
		static std::vector<vk::VertexInputBindingDescription> VertexBindings;
		static vk::PipelineVertexInputStateCreateInfo VertexState;

		static vk::PipelineVertexInputStateCreateInfo GetVertexState() {
			static bool first = true;
			if (first) {
				//bindings
				vk::VertexInputBindingDescription bindDesc;
				bindDesc.binding = 0;
				bindDesc.inputRate = vk::VertexInputRate::eVertex;
				bindDesc.stride = sizeof(glm::vec3);
				VertexBindings.push_back(bindDesc);
				bindDesc.binding = 1;
				bindDesc.inputRate = vk::VertexInputRate::eVertex;
				bindDesc.stride = sizeof(glm::vec3);
				VertexBindings.push_back(bindDesc);
				bindDesc.binding = 2;
				bindDesc.inputRate = vk::VertexInputRate::eVertex;
				bindDesc.stride = sizeof(glm::vec3);
				VertexBindings.push_back(bindDesc);
				bindDesc.binding = 3;
				bindDesc.inputRate = vk::VertexInputRate::eVertex;
				bindDesc.stride = sizeof(glm::vec2);
				VertexBindings.push_back(bindDesc);
				//input attributes
				vk::VertexInputAttributeDescription attrDesc;
				attrDesc.binding = 0;
				attrDesc.format = vk::Format::eR32G32B32Sfloat;
				attrDesc.location = 0;
				attrDesc.offset = 0;
				VertexAttributes.push_back(attrDesc);//posl
				attrDesc.binding = 1;
				attrDesc.format = vk::Format::eR32G32B32Sfloat;
				attrDesc.location = 1;
				attrDesc.offset = 0;
				VertexAttributes.push_back(attrDesc);//normal
				attrDesc.binding = 2;
				attrDesc.format = vk::Format::eR32G32B32Sfloat;
				attrDesc.location = 2;
				attrDesc.offset = 0;
				VertexAttributes.push_back(attrDesc); //tangent
				attrDesc.binding = 3;
				attrDesc.format = vk::Format::eR32G32Sfloat;
				attrDesc.location = 3;
				attrDesc.offset = 0;
				VertexAttributes.push_back(attrDesc); //texcoord

				VertexState.pVertexBindingDescriptions = VertexBindings.data();
				VertexState.vertexBindingDescriptionCount = (uint32_t)VertexBindings.size();
				VertexState.pVertexAttributeDescriptions = VertexAttributes.data();
				VertexState.vertexAttributeDescriptionCount = (uint32_t)VertexAttributes.size();
				first = false;
			}
			return VertexState;
		}
	}
}