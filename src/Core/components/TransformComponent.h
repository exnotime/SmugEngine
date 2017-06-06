#pragma once
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
struct TransformComponent {
	glm::vec3 Position;
	glm::vec3 Scale;
	glm::quat Orientation;
	glm::mat4 Transform;
	static unsigned int Flag;
};

static glm::quat EulertoQuaternion(float pitch, float roll, float yaw)
{
	glm::quat q;
	double t0 = std::cos(yaw * 0.5);
	double t1 = std::sin(yaw * 0.5);
	double t2 = std::cos(roll * 0.5);
	double t3 = std::sin(roll * 0.5);
	double t4 = std::cos(pitch * 0.5);
	double t5 = std::sin(pitch * 0.5);

	q.w = float(t0 * t2 * t4 + t1 * t3 * t5);
	q.x = float(t0 * t3 * t4 - t1 * t2 * t5);
	q.y = float(t0 * t2 * t5 + t1 * t3 * t4);
	q.z = float(t1 * t2 * t4 - t0 * t3 * t5);
	return q;
}