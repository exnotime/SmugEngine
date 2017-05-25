#pragma once
#include "PhysicsExport.h"
#include <PhysX/PxPhysicsAPI.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum PHYSICS_SHAPE {
	SPHERE,
	CUBE,
	PLANE,
	CAPSULE,
	SHAPE_COUNT
};

/*
Before physics update this is used to move kinematics and apply force on the body
After physics update this contains the updated position orientation of the actor shape
*/
struct PHYSICS_DLL PhysicsBody {
	glm::vec3 Position;
	glm::quat Orientation;
	glm::vec3 Force;
	uint32_t Actor;
};

class PHYSICS_DLL PhysicsEngine {
public:
	PhysicsEngine();
	~PhysicsEngine();

	void Init();
	void Update(double deltaTime);
	void Shutdown();

	PhysicsBody* CreateDynamicActor(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, float mass, PHYSICS_SHAPE shape, bool kinematic = false);
	PhysicsBody* CreateStaticActor(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, PHYSICS_SHAPE shape);
	void DeleteActor(uint32_t actor);

	void SetGravityPoint(const glm::vec3& pos, float strength);
private:
	physx::PxFoundation* m_Foundation;
	physx::PxPhysics* m_Physics;
	physx::PxScene* m_Scene;
#ifdef _DEBUG
	physx::PxPvd* m_PVD;
#endif

	physx::PxDefaultAllocator m_Allocator;
	physx::PxDefaultErrorCallback m_Error;

	float m_Accumulator = 0.0f;
	const float m_StepTime = 1.0f / 60.0f;

	std::vector<physx::PxRigidActor*> m_Actors;
	std::vector<PhysicsBody*> m_Bodies;
	physx::PxVec3 m_GravityPoint;
	float m_GravityFactor;
};