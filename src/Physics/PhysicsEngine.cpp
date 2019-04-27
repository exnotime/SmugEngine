#include "PhysicsEngine.h"
#include <glm/glm.hpp>
#include <PhysX4.0/common/PxPhysXCommonConfig.h>
#include <PhysX4.0/characterkinematic/PxControllerManager.h>

#if _DEBUG
#include <PhysX4.0/pvd/PxPvd.h>
#endif

#define GRAVITY_CONSTANT 9.82f

using namespace smug;
using namespace physx;

static PxDefaultAllocator g_DefaultAllocator;

class PhysXErrorReporter : public PxErrorCallback
{
public:
	PhysXErrorReporter(){}
	~PhysXErrorReporter(){}

	virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line) {
		printf(message);
	}
};

static PhysXErrorReporter g_ErrorHandler;

PhysicsEngine::PhysicsEngine() {

}
PhysicsEngine::~PhysicsEngine() {
	Shutdown();
}

void PhysicsEngine::Init() {
	m_Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, g_DefaultAllocator, m_Error);
	if (!m_Foundation) {
		printf("Error creating physx foundation\n");
		return;
	}

#ifdef _DEBUG
	//m_PVD = PxCreatePvd(*m_Foundation);
	//PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	//m_PVD->connect(*transport, PxPvdInstrumentationFlag::eALL);
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), false, nullptr);
#else
	m_Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_Foundation, PxTolerancesScale(), false, nullptr);
#endif

	if (!m_Physics) {
		printf("Error creating physx physics\n");
		return;
	}

	PxSceneDesc sceneDesc = PxSceneDesc(PxTolerancesScale());
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_Disbatcher = PxDefaultCpuDispatcherCreate(1);
	sceneDesc.cpuDispatcher = m_Disbatcher;
	sceneDesc.gravity = PxVec3(0,-9.2f,0);

	m_Scene = m_Physics->createScene(sceneDesc);
	if (!m_Scene) {
		printf("Error creating physx scene\n");
		return;
	}

	m_Accumulator = 0;

	PxCookingParams cookingParams = PxCookingParams(PxTolerancesScale());
	m_Cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_Foundation, cookingParams);

	m_ControllerManager = PxCreateControllerManager(*m_Scene);
}

void PhysicsEngine::Update(double deltaTime) {
	//add force on objects
	for (auto& body : m_Bodies) {
		if (body->Controller) {
			PxController* c = m_Controllers[body->Actor];
			if (body->Updated) {
				c->setPosition(PxExtendedVec3(body->Position.x, body->Position.y, body->Position.z));
				body->Updated = false;
			} else {
				PxControllerFilters filters;
				filters.mFilterCallback = nullptr;
				filters.mCCTFilterCallback = nullptr;
				filters.mFilterFlags = PxQueryFlag::eDYNAMIC | PxQueryFlag::eSTATIC;
				c->move(PxVec3(body->Force.x, body->Force.y, body->Force.z), 0.01f, deltaTime, filters);
			}
		}
		else {
			PxRigidDynamic* a = m_Actors[body->Actor]->is<PxRigidDynamic>();
			if (a) {
				a->addForce(PxVec3(body->Force.x, body->Force.y, body->Force.z));
				body->Force = glm::vec3(0);
				if (body->Kinematic && body->Updated) {
					a->setKinematicTarget(
						PxTransform(PxVec3(body->Position.x, body->Position.y, body->Position.z),
							PxQuat(body->Orientation.x, body->Orientation.y, body->Orientation.z, body->Orientation.w))
					);
					body->Updated = false;
				}
			}
		}
	}
	//step physics
	m_Accumulator += deltaTime;
	if (m_Accumulator <= m_StepTime)
		return;

	m_Accumulator -= m_StepTime;

	//add custom gravity
	if (m_HasActiveCustomGravity) {
		for (auto& actor : m_Actors) {
			PxRigidDynamic* d = actor->is<PxRigidDynamic>();
			if (d) {
				PxVec3 pos = d->getGlobalPose().p;
				PxVec3 dir = PxVec3(0);
				dir.x = m_GravityPoint.x - pos.x;
				dir.y = m_GravityPoint.y - pos.y;
				dir.z = m_GravityPoint.z - pos.z;
				d->addForce(dir.getNormalized() * GRAVITY_CONSTANT * m_GravityFactor);
			}
		}
	}

	m_Scene->simulate((PxReal)m_StepTime);
	m_Scene->fetchResults(true);

	//get updated positions
	for (auto& body : m_Bodies) {
		if (body->Controller) {
			PxController* c = m_Controllers[body->Actor];
			PxExtendedVec3 p = c->getPosition();
			body->Position = glm::vec3(p.x, p.y, p.z);
			PxControllerState state;
			c->getState(state);
			body->IsOnGround = state.collisionFlags & PxControllerCollisionFlag::eCOLLISION_DOWN;
		} else {
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
}

void PhysicsEngine::Shutdown() {
	m_Cooking->release();

	for (auto& c : m_Controllers) {
		c->release();
	}

	m_ControllerManager->purgeControllers();
	m_ControllerManager->release();
	m_ControllerManager = nullptr;

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
	//m_PVD->getTransport()->release();
	//m_PVD->release();
	//m_PVD = nullptr;
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
	transform.q.x = orientation.x;
	transform.q.y = orientation.y;
	transform.q.z = orientation.z;
	transform.q.w = orientation.w;

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
	if (!m_FreeActors.empty()) {
		m_Actors[m_FreeActors.back()] = actor;
		m_FreeActors.pop_back();
	}
	else {
		m_Actors.push_back(actor);
	}

	PhysicsBody* body = new PhysicsBody();
	body->Actor = (uint32_t)m_Actors.size() - 1;
	body->Position = pos;
	body->Orientation = orientation;
	body->Kinematic = kinematic;
	body->Updated = false;
	m_Actors[body->Actor]->userData = &body->UserData; //have this as a way to get back to entity when doing picking etc
	m_Bodies.push_back(body);
	return body;
}

PhysicsBody* PhysicsEngine::CreateStaticActor(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, PHYSICS_SHAPE shape) {
	PxTransform transform;
	transform.p.x = pos.x;
	transform.p.y = pos.y;
	transform.p.z = pos.z;
	transform.q.x = orientation.x;
	transform.q.y = orientation.y;
	transform.q.z = orientation.z;
	transform.q.w = orientation.w;

	
	if (shape == PLANE) {
		glm::vec3 d = glm::normalize(-pos);
		float dist = glm::length(pos);
		PxPlane p = PxPlane(d.x,d.y,d.z, dist);
		transform = PxTransformFromPlaneEquation(p);
	}

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
	case PLANE:
		pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxPlaneGeometry(), *mat);
		break;
	default:
		return nullptr;
		break;
	}

	m_Scene->addActor(*actor);
	if (!m_FreeActors.empty()) {
		m_Actors[m_FreeActors.back()] = actor;
		m_FreeActors.pop_back();
	} else {
		m_Actors.push_back(actor);
	}

	PhysicsBody* body = new PhysicsBody();
	body->Actor = uint32_t(m_Actors.size() - 1);
	body->Position = pos;
	body->Orientation = orientation;
	m_Actors[body->Actor]->userData = &body->UserData; //have this as a way to get back to entity when doing picking etc
	m_Bodies.push_back(body);

	return body;
}

PhysicsBody* PhysicsEngine::CreateStaticActorFromTriMesh(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, PhysicsMesh& mesh) {
	PxTransform transform;
	transform.p.x = pos.x;
	transform.p.y = pos.y;
	transform.p.z = pos.z;
	transform.q.x = orientation.x;
	transform.q.y = orientation.y;
	transform.q.z = orientation.z;
	transform.q.w = orientation.w;

	PxTriangleMeshDesc meshDesc;
	meshDesc.points.data = mesh.Vertices.data();
	meshDesc.points.count = mesh.Vertices.size();
	meshDesc.points.stride = sizeof(glm::vec3);
	meshDesc.triangles.count = mesh.Indices.size() / 3;
	meshDesc.triangles.data = mesh.Indices.data();
	meshDesc.triangles.stride = sizeof(uint32_t) * 3;
	meshDesc.materialIndices.data = nullptr;
	meshDesc.materialIndices.stride = 0;

	bool valid = meshDesc.isValid();

	PxTriangleMesh* triMesh = m_Cooking->createTriangleMesh(meshDesc, m_Physics->getPhysicsInsertionCallback());

	PxRigidStatic* actor = m_Physics->createRigidStatic(transform);
	PxMaterial* mat = m_Physics->createMaterial((PxReal)0.5, (PxReal)0.5, (PxReal)0.1);
	PxMeshScale scale = PxMeshScale(PxVec3(size.x, size.y, size.z));

	PxShape* pxshape = PxRigidActorExt::createExclusiveShape(*actor, PxTriangleMeshGeometry(triMesh,scale), *mat);

	m_Scene->addActor(*actor);
	if (!m_FreeActors.empty()) {
		m_Actors[m_FreeActors.back()] = actor;
		m_FreeActors.pop_back();
	}
	else {
		m_Actors.push_back(actor);
	}

	PhysicsBody* body = new PhysicsBody();
	body->Actor = uint32_t(m_Actors.size() - 1);
	body->Position = pos;
	body->Orientation = orientation;
	m_Actors[body->Actor]->userData = &body->UserData; //have this as a way to get back to entity when doing picking etc
	m_Bodies.push_back(body);

	return body;
}

PhysicsBody* PhysicsEngine::CreateController(const glm::vec3& pos, const glm::quat& orientation, const glm::vec3& size, PHYSICS_SHAPE shape) {
	if (shape == CAPSULE) {
		PxCapsuleControllerDesc desc = {};
		desc.setToDefault();
		desc.height = size.y;
		desc.radius = size.x;
		desc.stepOffset = size.x;
		desc.position = PxExtendedVec3(pos.x, pos.y, pos.z);
		PxMaterial* mat = m_Physics->createMaterial((PxReal)0.5, (PxReal)0.5, (PxReal)0.1);
		desc.material = mat;
		bool valid = desc.isValid();
		PxController* c = m_ControllerManager->createController(desc);
		m_Controllers.push_back(c);
	} else if (shape == CUBE) {

	}

	PhysicsBody* body = new PhysicsBody();
	body->Actor = uint32_t(m_Controllers.size() - 1);
	body->Position = pos;
	body->Orientation = orientation;
	body->Controller = true;
	m_Controllers[body->Actor]->setUserData(&body->UserData); //have this as a way to get back to entity when doing picking etc
	m_Bodies.push_back(body);

	return body;
}



void PhysicsEngine::DeleteActor(uint32_t actor) {
	if (m_Actors.size() < actor && m_Actors[actor]) {
		PxActor* a = m_Actors[actor];
		m_Scene->removeActor(*a, true);
		a->release();
		m_Actors[actor] = nullptr;
		m_FreeActors.push_back(actor);
	}
}

void PhysicsEngine::SetGravityPoint(const glm::vec3& pos, float strength) {
	if (strength < 0) {
		m_HasActiveCustomGravity = false;
		return;
	}
	m_GravityPoint = pos;
	m_GravityFactor = strength;
	m_HasActiveCustomGravity = true;
}

void PhysicsEngine::LockRotationAxes(PhysicsBody* body, const glm::bvec3& axes) {
	if (m_Actors.size() < body->Actor && m_Actors[body->Actor]) {
		if (m_Actors[body->Actor]->getType() == physx::PxActorType::eRIGID_DYNAMIC) {
			PxRigidDynamic* a = reinterpret_cast<PxRigidDynamic*>(m_Actors[body->Actor]);
			if (axes.x) {
				a->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
			}
			if (axes.y) {
				a->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true);
			}
			if (axes.z) {
				a->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
			}
		}
	}
}

bool PhysicsEngine::RayCast(glm::vec3& origin, glm::vec3& direction, float maxDistance, RayCastResult& result) {
	PxRaycastBuffer buffer;
	bool ret = m_Scene->raycast(PxVec3(origin.x, origin.y, origin.z), PxVec3(direction.x, direction.y, direction.z), maxDistance, buffer, PxHitFlag::eDEFAULT);
	if (ret) {
		if (buffer.hasBlock) {
			result.HitObject = *(uint32_t*)buffer.block.actor->userData;
			result.Distance = buffer.block.distance;
			result.Positon = glm::vec3(buffer.block.position.x, buffer.block.position.y, buffer.block.position.z);
			result.Normal = glm::vec3(buffer.block.normal.x, buffer.block.normal.y, buffer.block.normal.z);
		} else {
			//look for closest point
			float d = FLT_MAX;
			for (uint32_t i = 0; i < buffer.nbTouches; ++i) {
				PxRaycastHit& h = buffer.touches[i];
				if (h.distance < d) {
					d = h.distance;
					result.HitObject = *(uint32_t*)h.actor->userData;
					result.Distance = h.distance;
					result.Positon = glm::vec3(h.position.x, h.position.y, h.position.z);
					result.Normal = glm::vec3(h.normal.x, h.normal.y, h.normal.z);
				}
			}
		}
		
	}
	return ret;
}