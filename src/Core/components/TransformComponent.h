#pragma once
#include <glm/glm.hpp>
struct TransformComponent {
	glm::vec3 Position;
	glm::vec3 Scale;
	glm::quat Orientation;
	glm::mat4 Transform;
	static unsigned int Flag;
};

