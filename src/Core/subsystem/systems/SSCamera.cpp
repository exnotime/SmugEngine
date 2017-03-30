#include "SSCamera.h"
#include "Core/components/CameraComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"

#include "Core/Input.h"
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
			
			glm::dvec3 velocity = glm::vec3(0.0f);
			if (g_Input.IsKeyDown(GLFW_KEY_W)) {
				velocity += glm::dvec3(0, 0, 1);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_S)) {
				velocity += glm::dvec3(0, 0, -1);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_A)) {
				velocity += glm::dvec3(-1, 0, 0);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_D)) {
				velocity += glm::dvec3(1, 0, 0);
			}
			if (g_Input.IsKeyDown(GLFW_KEY_SPACE)) {
				cc->Cam.MoveWorld(glm::vec3(0, -1 * deltaTime, 0));
			}
			if (g_Input.IsKeyDown(GLFW_KEY_C)) {
				cc->Cam.MoveWorld(glm::vec3(0, 1 * deltaTime, 0));
			}
			cc->Cam.MoveRelative(velocity * deltaTime);
		}
	}

}

void SSCamera::Shutdown() {
}

