#include "SSCamera.h"
#include "Core/components/CameraComponent.h"
#include "Core/components/TransformComponent.h"
#include "Core/components/ModelComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/components/ScriptComponent.h"
#include "Core/entity/EntityManager.h"
#include "Core/Input.h"
#include <Graphics/GraphicsEngine.h>
#include <AssetLoader/AssetLoader.h>
#include <glm/gtx/transform.hpp>
#include "../../GlobalSystems.h"
#include "../../Camera.h"

using namespace smug;
#define CAMERA_SPEED 20.0f
#define SPRINT_FACTOR 5.0f
#define CAMERA_ROTATION_SPEED 0.001f
SSCamera::SSCamera() {

}

SSCamera::~SSCamera() {

}

void SSCamera::Startup() {

	//create camera entity
	Entity& e = globals::g_EntityManager->CreateEntity();

	TransformComponent tc;
	tc.Position = glm::vec3(0, 0, 0);
	tc.Scale = glm::vec3(1.0f);
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);

	CameraComponent cc;
	cc.Cam.SetPosition(tc.Position);
	globals::g_Components->CreateComponent(&cc, e, CameraComponent::Flag);

	ScriptComponent sc;
	CreateScriptComponent("script/DetectFunctionTest.as", sc);
	globals::g_Components->CreateComponent(&sc, e, ScriptComponent::Flag);

	m_Cache = new EntityCache();
	m_Cache->ComponentBitMask = e.ComponentBitfield;
	m_Cache->Entities.push_back(e.UID);

}

void SSCamera::Update(const double deltaTime) {
	using namespace glm;
	int flag = CameraComponent::Flag | TransformComponent::Flag;
	auto& entityManager = globals::g_EntityManager;
	if (!entityManager->IsCacheDirty(*m_Cache)) {
		uint32_t size = (uint32_t)m_Cache->Entities.size();
		for (uint32_t i = 0; i < size; ++i) {
			Entity& e = entityManager->GetEntity(m_Cache->Entities[i]);
			CameraComponent* cc = (CameraComponent*)globals::g_Components->GetComponent(e, CameraComponent::Flag);
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);

			glm::vec3 velocity = glm::vec3(0.0f);
			double speed = CAMERA_SPEED * deltaTime;
			if (g_Input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
				speed *= SPRINT_FACTOR;
			}

			if (g_Input.IsKeyDown(GLFW_KEY_W)) {
				cc->Cam.MoveRelative(glm::vec3(0, 0, -speed));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_S)) {
				cc->Cam.MoveRelative(glm::vec3(0, 0, speed));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_A)) {
				cc->Cam.MoveRelative(glm::vec3(-speed, 0, 0));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_D)) {
				cc->Cam.MoveRelative(glm::vec3(speed, 0, 0));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_SPACE)) {
				cc->Cam.MoveWorld(glm::vec3(0, speed, 0));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_C)) {
				cc->Cam.MoveWorld(glm::vec3(0, -speed, 0));
			}

			cc->Cam.YawWorld((float)g_Input.GetMouseDelta().x * -CAMERA_ROTATION_SPEED);
			cc->Cam.PitchRelative((float)g_Input.GetMouseDelta().y * CAMERA_ROTATION_SPEED);

			cc->Cam.CalculateViewProjection();

			globals::g_Gfx->GetRenderQueue()->AddCamera(cc->Cam.GetData());
		}
	}
}

void SSCamera::Shutdown() {
	delete m_Cache;
}

