#pragma once
/*
==============================================================================

   Basic camera functionality provided by the Graphics Engine.
   Author: Ola Enberg
   Modified by : Henrik Johansson

==============================================================================
*/

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS	// Needs to be included before any GLM stuff is loaded.
#endif
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>
#include <Graphics/RenderQueue.h> //CameraData
// Lens used for calculating the projection matrix of the camera(Camera) provided by the Graphics Engine.
struct CameraLens {
#ifdef GLM_FORCE_RADIANS
	float		VerticalFOV = 0.61f;	// Field Of View in screen y-dirction. Given in radians because GLM_FORCE_RADIANS is defined.
#else
	float		VerticalFOV		= 35.0f;	// Field Of View in screen y-dirction. Given in degrees because GLM_FORCE_RADIANS is NOT defined.
#endif
	int			WindowWidth = 1280;		// Width of view port.
	int			WindowHeight = 720;		// Height of view port.
	float		Near = 0.5f;		// Minimum distance from camera for rendered objects.
	float		Far = 1000.0f;	// Maximum distance from camera for rendered objects.
};

// Basic camera provided by the Graphics Engine. Should not be controlled directly by input devices (use inheritance or similar instead).
class Camera {
  public:
	  Camera();
	// Returns true if Camera passes all tests.
	static bool				UnitTest();

	// Dummy update to handle cameras generally
	virtual void 			Update(float deltaTime) { };

	// Should be called just before View or Projection is needed. Get result through GetView(), GetProjection(), and/or GetViewProjection().
	void					CalculateViewProjection();

	// Move camera along the worlds coordinate axises.
	void					MoveWorld(const glm::vec3& distanceToMove);

	// Move camera along the cameras base vectors. x is right, y up, and -z forward (GL Standard).
	void					MoveRelative(const glm::vec3& distanceToMove);

	// Rotate camera around world y-axis (up-vector).
	void					YawWorld(const float radians);

	// Rotate camera around the cameras up-vector.
	void					YawRelative(const float radians);

	// Rotate camera around world x-axis (right-vector).
	void					PitchWorld(const float radians);

	// Rotate camera around the cameras right-vector.
	void					PitchRelative(const float radians);

	// Rotate camera around world (minus)z-axis (forward-vector).
	void					RollWorld(const float radians);

	// Rotate camera around the cameras forward-vector.
	void					RollRelative(const float radians);

	// Rotate camera around a normalized axis given in world space coordinates. Alternative to using yaw, pitch, and roll.
	void					RotateAroundNormalizedAxis(const glm::vec3& normalizedRotationAxis, const float radians);

	// Provides full control of rotation. Only use if yaw, pitch, roll or rotate around axis isn't enough for your situation.
	void					RotateWithQuaternion(const glm::quat& rotation);

	// Camera basevector given in world space coordninates. WARNING: Does calculations. Call as few times as possible.
	const glm::vec3			GetForward() const;

	// Camera basevector given in world space coordninates. WARNING: Does calculations. Call as few times as possible.
	const glm::vec3			GetUp() const;

	// Camera basevector given in world space coordninates. WARNING: Does calculations. Call as few times as possible.
	const glm::vec3			GetRight() const;

	const glm::vec3&		GetPosition() const;
	const glm::quat&		GetOrientation() const;
	const glm::mat4&		GetView() const;
	const glm::mat4&		GetProjection() const;
	// WARNING: Does calculations. Call as few times as possible.
	const glm::mat4			GetViewProjection() const;
	const CameraData		GetData() const;

	glm::vec3&				GetEditablePosition();

	// Example of Use: GetEditableLens().FOV = 90.0f; Do not assign the entire lens to a local variable since that will create a copy.
	CameraData&				GetEditableData();

	void LookAt(const glm::vec3& position);

	void					SetPosition(const glm::vec3& newPosition);
	void					SetOrientation(const glm::quat& newOrientation);
	virtual void			SetMoveSpeed(const float newMoveSpeed);

  private:
	  CameraData m_CamData;
	  glm::quat m_Orientation;
};
