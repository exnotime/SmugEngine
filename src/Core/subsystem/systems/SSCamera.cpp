#include "SSCamera.h"
#include "Core/components/CameraComponent.h"
#include "Core/components/TransformComponent.h"
#include "Core/components/ModelComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/components/ScriptComponent.h"
#include "Core/components/RigidBodyComponent.h"
#include "Core/entity/EntityManager.h"
#include "Core/Input.h"
#include <Graphics/GraphicsEngine.h>
#include <AssetLoader/AssetLoader.h>
#include <glm/gtx/transform.hpp>
#include "../../GlobalSystems.h"
#include "../../Camera.h"

using namespace smug;
#define CAMERA_SPEED 2.5f
#define SPRINT_FACTOR 4.0f
#define MOUSE_ROTATION_SPEED 0.001f
#define GAMEPAD_ROTATION_SPEED 0.015f
SSCamera::SSCamera() {

}

SSCamera::~SSCamera() {

}

void SSCamera::Startup() {

	//create camera entity
	Entity& e = globals::g_EntityManager->CreateEntity();

	TransformComponent tc;
	tc.Position = glm::vec3(0, 10, -15);
	tc.Scale = glm::vec3(1.0f);
	globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);

	CameraComponent cc;
	cc.Cam.SetPosition(tc.Position);
	cc.Cam.GetEditableData().Far = 1000.0f;
	globals::g_Components->CreateComponent(&cc, e, CameraComponent::Flag);

	ScriptComponent sc;
	CreateScriptComponent("script/DetectFunctionTest.as", sc);
	globals::g_Components->CreateComponent(&sc, e, ScriptComponent::Flag);

	RigidBodyComponent rc;
	rc.Body =  globals::g_Physics->CreateController(tc.Position, tc.Orientation, glm::vec3(0.25f, 1.0f, 0.0f), CAPSULE);
	globals::g_Components->CreateComponent(&rc, e, RigidBodyComponent::Flag);

	m_Cache = new EntityCache();
	m_Cache->ComponentBitMask = e.ComponentBitfield;
	m_Cache->Entities.push_back(e.UID);

	m_JumpVelocity = 0.0f;
	m_JumpForce = 0.0f;

}

void SSCamera::Update(const double deltaTime) {
	using namespace glm;
	int flag = CameraComponent::Flag | TransformComponent::Flag | RigidBodyComponent::Flag;
	auto& entityManager = globals::g_EntityManager;
	if (!entityManager->IsCacheDirty(*m_Cache)) {
		uint32_t size = (uint32_t)m_Cache->Entities.size();
		for (uint32_t i = 0; i < size; ++i) {
			Entity& e = entityManager->GetEntity(m_Cache->Entities[i]);
			CameraComponent* cc = (CameraComponent*)globals::g_Components->GetComponent(e, CameraComponent::Flag);
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);
			RigidBodyComponent* rc = (RigidBodyComponent*)globals::g_Components->GetComponent(e, RigidBodyComponent::Flag);

			if (g_Input.IsKeyPushed(GLFW_KEY_F1)) {
				m_FreeFlight = !m_FreeFlight;
				if (!m_FreeFlight) {
					rc->Body->Position = cc->Cam.GetPosition();
					rc->Body->Updated = true;
				}
			}

			if (m_FreeFlight) {
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
				cc->Cam.YawWorld((float)g_Input.GetMouseDelta().x * MOUSE_ROTATION_SPEED  * -1.0f);
				cc->Cam.PitchRelative((float)g_Input.GetMouseDelta().y * MOUSE_ROTATION_SPEED);

				//gamepad
				Gamepad gp = g_Input.GetGamepadState(0);
				if (gp.LeftY > 0.2f) {
					cc->Cam.MoveRelative(glm::vec3(0, 0, -gp.LeftY * speed));
				}
				if (gp.LeftY < -0.2f) {
					cc->Cam.MoveRelative(glm::vec3(0, 0, -gp.LeftY * speed));
				}
				if (gp.LeftX > 0.2f) {
					cc->Cam.MoveRelative(glm::vec3(gp.LeftX * speed, 0, 0));
				}
				if (gp.LeftX < -0.2f) {
					cc->Cam.MoveRelative(glm::vec3(gp.LeftX * speed, 0, 0));
				}
				if (gp.RB) {
					cc->Cam.MoveWorld(glm::vec3(0, speed, 0));
				}
				if (gp.LB) {
					cc->Cam.MoveWorld(glm::vec3(0, -speed, 0));
				}

				if (abs(gp.RightX) > 0.1f) {
					cc->Cam.YawWorld(gp.RightX * GAMEPAD_ROTATION_SPEED * -1.0f);
				}
				if (abs(gp.RightY) > 0.1f) {
					cc->Cam.PitchRelative(gp.RightY * GAMEPAD_ROTATION_SPEED * -1.0f);
				}
			} else {
				//physics driven camera entity
				//rotate with mouse
				cc->Cam.YawWorld((float)g_Input.GetMouseDelta().x * MOUSE_ROTATION_SPEED  * -1.0f);
				cc->Cam.PitchRelative((float)g_Input.GetMouseDelta().y * MOUSE_ROTATION_SPEED);
				//calc velocity 
				glm::vec3 velocity = glm::vec3(0.0f);
				float speed = CAMERA_SPEED * deltaTime;

				if (g_Input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
					speed *= SPRINT_FACTOR;
				}
				glm::vec3 f = cc->Cam.GetForward();
				f.y = 0.0f;
				f = glm::normalize(f);

				glm::vec3 r = cc->Cam.GetRight();
				r.y = 0.0f;
				r = glm::normalize(r);

				if (g_Input.IsKeyDown(GLFW_KEY_W)) {
					velocity += f * speed;
				}
				if (g_Input.IsKeyDown(GLFW_KEY_S)) {
					velocity += f * -speed;
				}
				if (g_Input.IsKeyDown(GLFW_KEY_A)) {
					velocity += r * -speed;
				}
				if (g_Input.IsKeyDown(GLFW_KEY_D)) {
					velocity += r * speed;
				}
				if (g_Input.IsKeyPushed(GLFW_KEY_SPACE) && rc->Body->IsOnGround) {
					m_JumpVelocity = 5.0f;
				}
				m_JumpVelocity += -9.82f * deltaTime;

				//get last physics position
				cc->Cam.SetPosition(rc->Body->Position + glm::vec3(0, 0.5f,0));
				//update body
				rc->Body->Force = velocity + m_JumpVelocity * glm::vec3(0,1,0) * (float)deltaTime;
			}
			cc->Cam.CalculateViewProjection();

			globals::g_Gfx->GetRenderQueue()->AddCamera(cc->Cam.GetData());
		}
	}
}

void SSCamera::Shutdown() {
	delete m_Cache;
}

