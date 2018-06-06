#include "PhysicsEngine.h"
#include <glm/glm.hpp>

#if _DEBUG
#include <PhysX/pvd/PxPvd.h>
#endif
using namespace smug;
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

#ifdef _DEBUG
	m_PVD = PxCreatePvd(*m_Foundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), false, m_PVD);
#else
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), false, nullptr);
#endif

	if (!m_Physics) {
		printf("Error creating physx physics\n");
		return;
	}

	PxSceneDesc sceneDesc = PxSceneDesc(PxTolerancesScale());
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_Disbatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_Disbatcher;

	//bool isValid = sceneDesc.isValid();
	m_Scene = m_Physics->createScene(sceneDesc);
	if (!m_Scene) {
		printf("Error creating physx scene\n");
		return;
	}

	m_Accumulator = 0;
}

void PhysicsEngine::Update(double deltaTime) {
	//add force on objects
	for (auto& body : m_Bodies) {
		PxRigidDynamic* a = m_Actors[body->Actor]->is<PxRigidDynamic>();
		if (a) {
			a->addForce(PxVec3(body->Force.x, body->Force.y, body->Force.z));
			body->Force = glm::vec3(0);
		}
	}
	//step physics
	m_Accumulator += deltaTime;
	if (m_Accumulator <= m_StepTime)
		return;

	m_Accumulator -= m_StepTime;

	//add custom gravity
	for (auto& actor : m_Actors) {
		PxRigidDynamic* d = actor->is<PxRigidDynamic>();
		if (d) {
			PxVec3 pos = d->getGlobalPose().p;
			PxVec3 dir = m_GravityPoint - pos;
			d->addForce(dir.getNormalized() * 9.2f * m_GravityFactor);
		}
	}

	m_Scene->simulate((PxReal)m_StepTime);
	m_Scene->fetchResults(true);

	//get updated positions
	for (auto& body : m_Bodies) {
		PxRigidDynamic* d = m_Actors[body->Actor]->is<PxRigidDynamic>();
		if (d) {
			if (!d->isSleeping()) {
				PxTransform t = d->getGlobalPose();
				body->Position = glm::vec3(t.p.x, t.p.y, t.p.z);
				body->Orientation = glm::quat(t.q.w, t.q.x, t.q.y, t.q.z);
			}
		}
	}
}

void PhysicsEngine::Shutdown() {
	m_Disbatcher->release();
	m_Disbatcher = nullptr;

	for (auto& a : m_Actors) {
		m_Scene->removeActor(*a);
	}
	m_Actors.clear();

	m_Scene->release();
	m_Scene = nullptr;

	m_Physics->release();
	m_Physics = nullptr;

#ifdef _DEBUG
	m_PVD->getTransport()->release();
	m_PVD->release();
	m_PVD = nullptr;
#endif

	m_Foundation->release();
	m_Foundation = nullptr;

	for (auto& body : m_Bodies) {
		delete body;
	}
	m_Bodies.clear();
}

PhysicsBody* PhysicsEngine::CreateDynamicActor(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, float mass, PHYSICS_SHAPE shape, bool kinematic) {
	PxTransform transform;
	transform.p.x = pos.x;
	transform.p.y = pos.y;
	transform.p.z = pos.z;
	transform.q.x = orientation.y;
	transform.q.y = orientation.z;
	transform.q.z = orientation.w;
	transform.q.w = orientation.x;

	PxRigidDynamic* actor = m_Physics->createRigidDynamic(transform);
	PxMaterial* mat = m_Physics->createMaterial((PxReal)0.5, (PxReal)0.5, (PxReal)0.1);
	PxShape* pxshape;
	switch (shape) {
	case SPHERE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxSphereGeometry(size.x), *mat);
		break;
	case CUBE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxBoxGeometry(size.x, size.y,size.z), *mat);
		break;
	case CAPSULE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxCapsuleGeometry(size.x,size.y), *mat);
		break;
	default:
		return nullptr;
		break;
	}

	PxRigidBodyExt::updateMassAndInertia(*actor, mass);
	actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);

	m_Scene->addActor(*actor);
	m_Actors.push_back(actor);

	PhysicsBody* body = new PhysicsBody();
	body->Actor = (uint32_t)m_Actors.size() - 1;
	body->Position = pos;
	body->Orientation = orientation;
	m_Bodies.push_back(body);

	return body;
}

PhysicsBody* PhysicsEngine::CreateStaticActor(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, PHYSICS_SHAPE shape) {
	PxTransform transform;
	transform.p.x = pos.x;
	transform.p.y = pos.y;
	transform.p.z = pos.z;
	transform.q.x = orientation.y;
	transform.q.y = orientation.z;
	transform.q.z = orientation.w;
	transform.q.w = orientation.x;

	PxRigidStatic* actor = m_Physics->createRigidStatic(transform);
	PxMaterial* mat = m_Physics->createMaterial((PxReal)0.5, (PxReal)0.5, (PxReal)0.1);
	PxShape* pxshape;
	switch (shape) {
	case SPHERE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxSphereGeometry(size.x), *mat);
		break;
	case CUBE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxBoxGeometry(size.x, size.y, size.z), *mat);
		break;
	case CAPSULE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxCapsuleGeometry(size.x, size.y), *mat);
		break;
	default:
		return nullptr;
		break;
	}

	m_Scene->addActor(*actor);
	m_Actors.push_back(actor);

	PhysicsBody* body = new PhysicsBody();
	body->Actor = uint32_t(m_Actors.size() - 1);
	body->Position = pos;
	body->Orientation = orientation;
	m_Bodies.push_back(body);

	return body;
}

void PhysicsEngine::DeleteActor(uint32_t actor) {

}

void PhysicsEngine::SetGravityPoint(const glm::vec3& pos, float strength) {
	m_GravityPoint = PxVec3(pos.x,pos.y,pos.z);
	m_GravityFactor = strength;
}