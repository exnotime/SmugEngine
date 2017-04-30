#include "SSCamera.h"
#include "Core/components/CameraComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"
#include "Core/Input.h"
#include <Graphics/GraphicsEngine.h>
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
}

void SSCamera::Update(const double deltaTime) {
	int flag = CameraComponent::Flag;

	for (auto& e : g_EntityManager.GetEntityList()) {
		if (e.ComponentBitfield & flag) {
			CameraComponent* cc = (CameraComponent*)g_ComponentManager.GetComponent(e, CameraComponent::Flag);
			
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
}

void SSCamera::Shutdown() {
}
