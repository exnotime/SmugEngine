#include "Camera.h"
#include <glm/gtx/transform2.hpp>	// lookAt, and perspectiveFov
Camera::Camera() {
	m_CamData.View = glm::mat4(1);
	m_CamData.Proj = glm::mat4(1);
	m_CamData.ProjView = glm::mat4(1);
	m_CamData.Position = glm::vec3(0);
	m_CamData.Forward = glm::vec3(0, 0, -1);
	m_CamData.Right = glm::vec3(1, 0, 0);
	m_CamData.Up = glm::vec3(0, 1, 0);
	m_CamData.Fov = 0.61f;
	m_CamData.Near = 0.1f;
	m_CamData.Far = 1000.0f;
	m_CamData.Width = 16;
	m_CamData.Height = 9;
	m_Orientation = glm::quat(1, 0, 0, 0);
}
void Camera::CalculateViewProjection() {
	m_Orientation	= glm::normalize( m_Orientation );
	m_CamData.View	= glm::lookAt(m_CamData.Position, m_CamData.Position + this->GetForward(), this->GetUp() );
	m_CamData.Proj	= glm::perspective(m_CamData.Fov, m_CamData.Width / static_cast<float>(m_CamData.Height), m_CamData.Near, m_CamData.Far );
	m_CamData.ProjView = m_CamData.Proj * m_CamData.View;
}

void Camera::MoveWorld(const glm::vec3& distanceToMove) {
	m_CamData.Position += distanceToMove;
}

void Camera::MoveRelative(const glm::vec3& distanceToMove) {
	m_CamData.Position += distanceToMove.x * (m_Orientation * m_CamData.Right	);
	m_CamData.Position += distanceToMove.y * (m_Orientation * m_CamData.Up		);
	m_CamData.Position -= distanceToMove.z * (m_Orientation * m_CamData.Forward	);
}

void Camera::YawWorld(const float radians) {
	RotateAroundNormalizedAxis(m_CamData.Up, radians );
}

void Camera::YawRelative(const float radians) {
	RotateAroundNormalizedAxis( m_Orientation * m_CamData.Up, radians );
}

void Camera::PitchWorld(const float radians) {
	RotateAroundNormalizedAxis(m_CamData.Right, radians );
}

void Camera::PitchRelative(const float radians) {
	RotateAroundNormalizedAxis( m_Orientation * m_CamData.Right, radians );
}

void Camera::RollWorld(const float radians) {
	RotateAroundNormalizedAxis(m_CamData.Forward, radians );
}

void Camera::RollRelative(const float radians) {
	RotateAroundNormalizedAxis( m_Orientation * m_CamData.Forward, radians );
}

void Camera::RotateAroundNormalizedAxis(const glm::vec3& normalizedRotationAxis, const float radians) {
	float rotationAmount = radians * 0.5f;
	glm::quat rotation( glm::cos(rotationAmount), normalizedRotationAxis * glm::sin(rotationAmount) );
	m_Orientation = glm::normalize( rotation * m_Orientation );
}

void Camera::RotateWithQuaternion(const glm::quat& rotation) {
	m_Orientation = glm::normalize( rotation * m_Orientation );
}

const glm::vec3 Camera::GetForward() const {
	return m_Orientation * m_CamData.Forward;
}

const glm::vec3 Camera::GetUp() const {
	return m_Orientation * m_CamData.Up;
}

const glm::vec3 Camera::GetRight() const {
	return m_Orientation * m_CamData.Right;
}

const glm::vec3& Camera::GetPosition() const {
	return m_CamData.Position;
}

const glm::quat& Camera::GetOrientation() const {
	return m_Orientation;
}

const glm::mat4& Camera::GetView() const {
	return m_CamData.View;
}

const glm::mat4& Camera::GetProjection() const {
	return m_CamData.Proj;
}

const glm::mat4 Camera::GetViewProjection() const {
	return m_CamData.Proj * m_CamData.View;
}

const CameraData Camera::GetData() const {
	CameraData cd;
	cd = m_CamData;
	cd.Right = GetRight();
	cd.Forward = GetForward();
	cd.Up = GetUp();
	return cd;
}

glm::vec3& Camera::GetEditablePosition() {
	return m_CamData.Position;
}

CameraData& Camera::GetEditableData() {
	return m_CamData;
}

void Camera::LookAt(const glm::vec3& position) {
	m_Orientation = glm::conjugate(glm::quat_cast(glm::lookAt(m_CamData.Position, position, glm::vec3(0,1,0))));
}

void Camera::SetPosition(const glm::vec3& newPosition) {
	m_CamData.Position = newPosition;
}

void Camera::SetOrientation(const glm::quat& newOrientation) {
	m_Orientation = newOrientation;
}

void Camera::SetMoveSpeed(const float newMoveSpeed) {
	// Just here to be overriden by its derived classes. But I don't want to force them to have an own implementation.
}