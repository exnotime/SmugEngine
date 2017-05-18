#include "SSCamera.h"
#include "Core/components/CameraComponent.h"
#include "Core/components/TransformComponent.h"
#include "Core/components/ModelComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"
#include "Core/Input.h"
#include <Graphics/GraphicsEngine.h>
#include <AssetLoader/AssetLoader.h>
#include <glm/gtx/transform.hpp>
#include "../../GlobalSystems.h"
#include "../../Camera.h"


#define CAMERA_SPEED 20.0f
#define SPRINT_FACTOR 5.0f
#define CAMERA_ROTATION_SPEED 0.001f
SSCamera::SSCamera(){

}

SSCamera::~SSCamera(){

}

void SSCamera::Startup() {
	//create camera entity
	Entity& e = g_EntityManager.CreateEntity();
	CameraComponent cc;
	g_ComponentManager.CreateComponent(&cc, e, CameraComponent::Flag);

	TransformComponent tc;
	tc.Position = glm::vec3(-1, 1, 2);
	tc.Scale = glm::vec3(1.0f);
	g_ComponentManager.CreateComponent(&tc, e, TransformComponent::Flag);

	ModelComponent mc;
	mc.ModelHandle = g_AssetLoader.LoadAsset("assets/suzanne.dae");
	g_ComponentManager.CreateComponent(&mc, e, ModelComponent::Flag);
}

glm::quat RotateAroundNormalizedAxis(float radians, glm::vec3 normalizedRotationAxis, glm::quat orientation) {
	float rotationAmount = radians * 0.5f;
	glm::quat rotation(glm::cos(rotationAmount), normalizedRotationAxis * glm::sin(rotationAmount));
	return glm::normalize(rotation * orientation);
}

void SSCamera::Update(const double deltaTime) {
	using namespace glm;
	int flag = CameraComponent::Flag | TransformComponent::Flag | ModelComponent::Flag;
	for (auto& e : g_EntityManager.GetEntityList()) {
		if ((e.ComponentBitfield & flag) == flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(e, CameraComponent::Flag);
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(e, TransformComponent::Flag);

			//position on sphere
			glm::vec3 movement = vec3(0);

			if (g_Input.IsKeyDown(GLFW_KEY_UP)) {
				movement += vec3(0, 0, 1);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_DOWN)) {
				movement += vec3(0, 0, -1);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_LEFT)) {
				movement += vec3(1, 0, 0);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_RIGHT)) {
				movement += vec3(-1, 0, 0);
			}

			vec3 spherePos = vec3(0, 20, 0);
			vec3 n = normalize(tc->Position - spherePos);
			n = normalize(quat(cos(movement.x * 0.01f), vec3(0, 1, 0) * sin(movement.x * 0.01f)) * n);
			n = normalize(quat(cos(movement.z * 0.01f), vec3(1, 0, 0) * sin(movement.z * 0.01f)) * n);
			tc->Position = spherePos + n * 10.8f;

			glm::vec3 velocity = glm::vec3(0.0f);
			float speed = CAMERA_SPEED * deltaTime;
			if(g_Input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
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

			cc->Cam.YawWorld(g_Input.GetMouseDelta().x * -CAMERA_ROTATION_SPEED);
			cc->Cam.PitchRelative(g_Input.GetMouseDelta().y * CAMERA_ROTATION_SPEED);

			cc->Cam.CalculateViewProjection();

			globals::g_Gfx->GetRenderQueue()->AddCamera(cc->Cam.GetData());
		}
	}
	//for (auto& e : g_EntityManager.GetEntityList()) {
	//	if (e.ComponentBitfield & flag) {
	//		CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(e, CameraComponent::Flag);
	//		

	//	}
	//}


}

void SSCamera::Shutdown() {
}

