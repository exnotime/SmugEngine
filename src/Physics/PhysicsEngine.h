#pragma once
#include "PhysicsExport.h"
#include <PhysX/PxPhysicsAPI.h>
#include <vector>

class PHYSICS_DLL PhysicsEngine {
public:
	PhysicsEngine();
	~PhysicsEngine();

	void Init();
	void Update(double deltaTime);
	void Shutdown();

	void CreateRigidActor();
	void DeleteRigidActor();
private:
	physx::PxFoundation* m_Foundation;
	physx::PxPhysics* m_Physics;
	physx::PxScene* m_Scene;

	physx::PxDefaultAllocator m_Allocator;
	physx::PxDefaultErrorCallback m_Error;

	float m_Accumulator = 0.0f;
	const float m_StepTime = 1.0f / 60.0f;

	std::vector<physx::PxRigidActor*> m_Actors;
};