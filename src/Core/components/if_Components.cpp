#include "if_Components.h"
#include <Core/script/ScriptEngine.h>
#include <Core/entity/EntityManager.h>
#include <Core/GlobalSystems.h>
#include "TransformComponent.h"
#include "ModelComponent.h"
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
	glm::quat& GetRotation(uint32_t euid){
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
		tc.Scale = glm::vec3(100);
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
		engine->RegisterGlobalFunction("void CreateModelComponent(uint euid, uint64 resource)", asFUNCTIONPR(CreateModelComponent, (uint32_t euid, uint64_t mr), void), asCALL_CDECL);
	}

}
