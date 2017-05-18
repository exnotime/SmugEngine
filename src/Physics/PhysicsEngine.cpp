#include "PhysicsEngine.h"
#include <glm/glm.hpp>
using namespace physx;

PhysicsEngine::PhysicsEngine() {

}
PhysicsEngine::~PhysicsEngine() {
	Shutdown();
}

void PhysicsEngine::Init() {

	m_Foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, m_Allocator, m_Error);
	if (!m_Foundation) {
		printf("Error creating physx foundation\n");
		return;
	}

	PxTolerancesScale toleranses;

	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, toleranses);
	if (!m_Physics) {
		printf("Error creating physx physics\n");
		return;
	}

	PxSceneDesc sceneDesc(toleranses);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(4);

	sceneDesc.isValid();
	m_Scene = m_Physics->createScene(sceneDesc);
	if (!m_Scene) {
		printf("Error creating physx scene\n");
		return;
	}

	m_Scene->setGravity(PxVec3(0, 9.2, 0));

	PxRigidStatic* staticPlane = m_Physics->createRigidStatic(PxTransformFromPlaneEquation(PxPlane(0,-1,0,0)));
	PxMaterial* mat = m_Physics->createMaterial(0.5, 0.5, 0.6);
	PxShape* planeShape = PxRigidActorExt::createExclusiveShape(*staticPlane, PxPlaneGeometry(), *mat);
	m_Scene->addActor(*staticPlane);

	m_Accumulator = 0;
}

void PhysicsEngine::Update(double deltaTime) {
	m_Accumulator += deltaTime;
	if (m_Accumulator <= m_StepTime)
		return;

	m_Accumulator -= m_StepTime;
	m_Scene->simulate(m_StepTime);
	m_Scene->fetchResults(true);

	for (auto& actor : m_Actors) {
		PxTransform t = actor->getGlobalPose();
		glm::vec3 pos = glm::vec3(t.p.x, t.p.y, t.p.z);
		int i = 0;
	}

}

void PhysicsEngine::Shutdown() {
	m_Scene->release(); m_Scene = nullptr;
	m_Physics->release(); m_Physics = nullptr;
	m_Foundation->release(); m_Foundation = nullptr;
}

void PhysicsEngine::CreateRigidActor() {
	PxRigidDynamic* dynamic = m_Physics->createRigidDynamic(PxTransform(PxVec3(0, -10, 0)));
	PxMaterial* mat = m_Physics->createMaterial(0.5, 0.5, 0.6);
	PxShape* sphereShape = PxRigidActorExt::createExclusiveShape(*dynamic, PxSphereGeometry(1.0), *mat);
	PxRigidBodyExt::updateMassAndInertia(*dynamic, 2.0f);
	m_Scene->addActor(*dynamic);
	m_Actors.push_back(dynamic);
}

void PhysicsEngine::DeleteRigidActor() {

}