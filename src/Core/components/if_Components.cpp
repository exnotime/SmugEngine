#include "if_Components.h"
#include <Core/script/ScriptEngine.h>
#include <Core/datasystem/ComponentManager.h>
#include <Core/entity/EntityManager.h>
#include <Core/GlobalSystems.h>
#include "TransformComponent.h"
#include "ModelComponent.h"
#include "RigidBodyComponent.h"

namespace smug {
namespace if_component {
//Transform component
#pragma region TransformComponent
TransformComponent* GetTransformComponent(uint32_t euid) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	if (e.ComponentBitfield & TransformComponent::Flag) {
		return (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);
	}
	return nullptr;
}

glm::vec3& GetPosition(uint32_t euid) {
	return GetTransformComponent(euid)->Position;
}
glm::vec3& GetScale(uint32_t euid) {
	return GetTransformComponent(euid)->Scale;
}
glm::quat& GetRotation(uint32_t euid) {
	return GetTransformComponent(euid)->Orientation;
}
glm::vec3 GetEulerRotation(uint32_t euid) {
	return glm::eulerAngles(GetTransformComponent(euid)->Orientation);
}

void CreateTransformComponent(uint32_t euid) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	TransformComponent tc;
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);
}

void CreateTransformComponent(uint32_t euid, glm::vec3 position) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = glm::angleAxis(0.0f, glm::vec3(0, 0, -1));
	tc.Scale = glm::vec3(1);
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);
}

void CreateTransformComponent(uint32_t euid, glm::vec3 position, glm::vec3 scale) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = glm::angleAxis(0.0f, glm::vec3(0, 0, -1));
	tc.Scale = scale;
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);
}

void CreateTransformComponent(uint32_t euid, glm::vec3 position, glm::vec3 scale, glm::quat rot) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	TransformComponent tc;
	tc.Position = position;
	tc.Orientation = rot;
	tc.Scale = scale;
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);
}

#pragma endregion

#pragma region ModelComponent
void CreateModelComponent(uint32_t euid, uint64_t modelResource) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	ModelComponent mc;
	mc.ModelHandle = modelResource;
	mc.Tint = glm::vec4(1);
	mc.Static = false;
	globals::g_Components->CreateComponent(&mc, e, ModelComponent::Flag);
}
#pragma endregion

#pragma region PhysicsComponent
void CreateRigidBodyComponent(uint32_t euid, uint32_t shape, float mass, bool staticActor, bool useTransform) {
	Entity& e = globals::g_EntityManager->GetEntity(euid);
	glm::vec3 pos = glm::vec3();
	glm::vec3 size = glm::vec3();
	glm::quat rot = glm::quat();
	if (e.ComponentBitfield & TransformComponent::Flag && useTransform) {
		TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);
		pos = tc->Position;
		rot = tc->Orientation;
		size = tc->Scale;
	}
	
	RigidBodyComponent rc;
	if (staticActor) {
		rc.Body = globals::g_Physics->CreateStaticActor(pos, rot, size, (PHYSICS_SHAPE)shape);
	}
	else {
		rc.Body = globals::g_Physics->CreateDynamicActor(pos, rot, size, mass, (PHYSICS_SHAPE)shape);
	}
	globals::g_Components->CreateComponent(&rc, e, RigidBodyComponent::Flag);
}
#pragma endregion

void InitComponentInterface() {
	using namespace AngelScript;

	asIScriptEngine* engine = g_ScriptEngine.GetEngine();
	engine->RegisterObjectType("TransformComponent", sizeof(TransformComponent), asOBJ_VALUE | asGetTypeTraits<TransformComponent>() | asOBJ_POD);
	engine->RegisterObjectProperty("TransformComponent", "vec3 Position", offsetof(TransformComponent, Position));
	engine->RegisterObjectProperty("TransformComponent", "vec3 Scale", offsetof(TransformComponent, Scale));
	engine->RegisterObjectProperty("TransformComponent", "quat Orientation", offsetof(TransformComponent, Orientation));
	engine->RegisterObjectProperty("TransformComponent", "mat4x4 Transform", offsetof(TransformComponent, Transform));
	engine->RegisterGlobalFunction("void CreateTransformComponent(uint euid)", asFUNCTIONPR(CreateTransformComponent, (uint32_t euid), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void CreateTransformComponent(uint euid, vec3 position)", asFUNCTIONPR(CreateTransformComponent, (uint32_t euid, glm::vec3 position), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void CreateTransformComponent(uint euid, vec3 position, vec3 scale)", asFUNCTIONPR(CreateTransformComponent, (uint32_t euid, glm::vec3 position, glm::vec3 scale), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void CreateTransformComponent(uint euid, vec3 position, vec3 scale, quat rotation)", asFUNCTIONPR(CreateTransformComponent, (uint32_t euid, glm::vec3 position, glm::vec3 scale, glm::quat rot), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void CreateModelComponent(uint euid, uint64 resource)", asFUNCTIONPR(CreateModelComponent, (uint32_t euid, uint64_t mr), void), asCALL_CDECL);
	engine->RegisterGlobalFunction("void CreateRigidBodyComponent(uint euid, uint shape, float mass, bool staticActor, bool useTransform)", asFUNCTIONPR(CreateRigidBodyComponent, (uint32_t euid, uint32_t shape, float mass, bool s, bool t), void), asCALL_CDECL);
	engine->RegisterEnum("PHYSICS_SHAPE");
	engine->RegisterEnumValue("PHYSICS_SHAPE", "SPHERE", PHYSICS_SHAPE::SPHERE);
	engine->RegisterEnumValue("PHYSICS_SHAPE", "CAPSULE", PHYSICS_SHAPE::CAPSULE);
	engine->RegisterEnumValue("PHYSICS_SHAPE", "PLANE", PHYSICS_SHAPE::PLANE);
	engine->RegisterEnumValue("PHYSICS_SHAPE", "CUBE", PHYSICS_SHAPE::CUBE);
}
}
}
