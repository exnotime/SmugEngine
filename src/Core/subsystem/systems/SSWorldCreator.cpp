#include "SSWorldCreator.h"
#include <Core/entity/EntityManager.h>
#include <Core/Input.h>
#include <Imgui/imgui.h>
#include <Core/components/CameraComponent.h>
#include <Core/components/TransformComponent.h>
#include <Core/components/ModelComponent.h>
#include <Core/datasystem/ComponentManager.h>
#include <Core/components/ScriptComponent.h>
#include <Core/components/RigidBodyComponent.h>
#include <Core/entity/EntityManager.h>
#include <Core/GlobalSystems.h>
#include <glm/gtx/transform.hpp>

using namespace smug;

SSWorldCreator::SSWorldCreator(){

}

SSWorldCreator::~SSWorldCreator(){

}

void SSWorldCreator::Startup() {
	m_Cache = new EntityCache();
	m_Cache->ComponentBitMask = CameraComponent::Flag | TransformComponent::Flag | RigidBodyComponent::Flag;
	m_Cache->Dirty = true;

	ResourceHandle model = g_AssetLoader.LoadAsset("assets/models/arrow/arrow.dae");
	//X
	Entity& xAxisEntity = globals::g_EntityManager->CreateEntity();
	TransformComponent tc;
	tc.Scale = glm::vec3(1.0f);
	tc.Position = glm::vec3(10, 5, 5);
	tc.Orientation = glm::rotate(glm::pi<float>() * 0.5f, glm::vec3(0, 1, 0));
	globals::g_Components->CreateComponent(&tc, xAxisEntity, TransformComponent::Flag);
	ModelComponent mc;
	mc.ModelHandle = model;
	mc.Static = false;
	mc.Tint = glm::vec4(1, 0, 0, 1);
	globals::g_Components->CreateComponent(&mc, xAxisEntity, ModelComponent::Flag);
	m_ArrowEntities[X] = xAxisEntity;
	//Y
	Entity& yAxisEntity = globals::g_EntityManager->CreateEntity();
	tc.Orientation = glm::rotate(-glm::pi<float>() * 0.5f, glm::vec3(1, 0, 0));
	globals::g_Components->CreateComponent(&tc, yAxisEntity, TransformComponent::Flag);
	mc.Tint = glm::vec4(0, 1, 0, 1);
	globals::g_Components->CreateComponent(&mc, yAxisEntity, ModelComponent::Flag);
	m_ArrowEntities[Y] = yAxisEntity;
	//Z
	Entity& zAxisEntity = globals::g_EntityManager->CreateEntity();
	tc.Orientation = glm::quat(); //The model is oriented towards Z+
	globals::g_Components->CreateComponent(&tc, zAxisEntity, TransformComponent::Flag);
	mc.Tint = glm::vec4(0, 0, 1, 1);
	globals::g_Components->CreateComponent(&mc, zAxisEntity, ModelComponent::Flag);
	m_ArrowEntities[Z] = zAxisEntity;
}

void SSWorldCreator::PickFromCamera(const glm::vec2& pos){
	ImVec2 screenSize = ImGui::GetIO().DisplaySize;
	glm::vec2 clipCoords = glm::vec2(pos.x / screenSize.x, pos.y / screenSize.y);
	clipCoords = (clipCoords * 2.0f) - 1.0f;
	int flag = CameraComponent::Flag | TransformComponent::Flag | RigidBodyComponent::Flag;
	auto& entityManager = globals::g_EntityManager;
	if (m_Cache->Dirty || entityManager->IsCacheDirty(*m_Cache)) {
		entityManager->UpdateCache(*m_Cache);
	}
	uint32_t size = (uint32_t)m_Cache->Entities.size();
	for (uint32_t i = 0; i < size; ++i) {
		Entity& e = entityManager->GetEntity(m_Cache->Entities[i]);
		CameraComponent* cc = (CameraComponent*)globals::g_Components->GetComponent(e, CameraComponent::Flag);

		const glm::mat4& invProjView = glm::inverse(cc->Cam.GetData().ProjView);
		//Calc ray origin
		glm::vec4 projOrigin = glm::vec4(invProjView * glm::vec4(clipCoords, 0.0f, 1));
		glm::vec3 origin = projOrigin / projOrigin.w;
		glm::vec3 dir = glm::normalize(origin - cc->Cam.GetData().Position);
		origin += dir * 0.25f;
		RayCastResult res;
		if (globals::g_Physics->RayCast(origin, dir, cc->Cam.GetData().Far, res)) {
			Entity& hitEntity = entityManager->GetEntity(res.HitObject);
			/*if (hitEntity.ComponentBitfield & ModelComponent::Flag) {
				ModelComponent* mc = (ModelComponent*)globals::g_Components->GetComponent(hitEntity, ModelComponent::Flag);
				mc->Tint = glm::vec4(1, 0, 0, 1);
			}*/

			m_SelectedEntity = &hitEntity;
		}
	}
}

void SSWorldCreator::Update(const double deltaTime) {
	//picking
	//only screen pick if we are not hovering on any window.
	if(!ImGui::IsAnyWindowHovered() && ImGui::IsMouseClicked(0) ){
		ImVec2 screenPos = ImGui::GetMousePos();
		PickFromCamera(glm::vec2(screenPos.x, screenPos.y));
	}
	if (m_SelectedEntity) {
		glm::vec3 selectedPos = ((TransformComponent*)globals::g_Components->GetComponent(*m_SelectedEntity, TransformComponent::Flag))->Position;

		for (uint32_t i = 0; i < NUM_AXES; ++i) {
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(m_ArrowEntities[i], TransformComponent::Flag);
			tc->Position = selectedPos;
		}
	}
	

}

void SSWorldCreator::Shutdown() {
	delete m_Cache;
}

