#include "SSPhysics.h"
#include <Core/GlobalSystems.h>
#include <Core/components/TransformComponent.h>
#include <Core/components/RigidBodyComponent.h>
#include <Core/components/ModelComponent.h>
#include <Core/components/CameraComponent.h>
#include <Core/datasystem/ComponentManager.h>
#include <Core/entity/EntityManager.h>
#include <Core/Input.h>

using namespace smug;
SSPhysics::SSPhysics() {

}

SSPhysics::~SSPhysics() {

}

void SSPhysics::Startup() {
}

void SSPhysics::Update(const double deltaTime) {
	//tick physics
	globals::g_Physics->Update(deltaTime);

	//fetch results
	int flag = TransformComponent::Flag | RigidBodyComponent::Flag;
	int cameraFlag = CameraComponent::Flag;
	auto& entities = globals::g_EntityManager->GetEntityList();
	uint32_t count = entities.size();
	for (uint32_t i = 0; i < count; ++i) {
		Entity& e = entities[i];
		if ((e.ComponentBitfield & flag) == flag) {
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);
			RigidBodyComponent* rc = (RigidBodyComponent*)globals::g_Components->GetComponent(e, RigidBodyComponent::Flag);

			tc->Position = rc->Body->Position;
			tc->Orientation = rc->Body->Orientation;
		}
		if ((e.ComponentBitfield & cameraFlag) == cameraFlag) {
			CameraComponent* cc = (CameraComponent*)globals::g_Components->GetComponent(e, CameraComponent::Flag);
			if (g_Input.IsMousebuttonPushed(GLFW_MOUSE_BUTTON_1)) {
				glm::vec3 forward = cc->Cam.GetForward();
				glm::vec3 pos = cc->Cam.GetPosition();

				Entity& e = globals::g_EntityManager->CreateEntity();
				TransformComponent tc;
				tc.Position = pos + forward * 2.0f;
				tc.Scale = glm::vec3(0.5f);
				tc.Orientation = glm::quat();
				globals::g_Components->CreateComponent(&tc, e, tc.Flag);
				RigidBodyComponent rc;
				rc.Body = globals::g_Physics->CreateDynamicActor(tc.Position, tc.Orientation, tc.Scale, 10.0f, PHYSICS_SHAPE::SPHERE);
				rc.Body->Force = forward * 10000.0f;
				globals::g_Components->CreateComponent(&rc, e, rc.Flag);
				ModelComponent mc;
				mc.ModelHandle = g_AssetLoader.LoadAsset("assets/models/sphere/sphere.obj");
				mc.Tint = glm::vec4(1.0f, 0, 0, 1.0f);
				globals::g_Components->CreateComponent(&mc, e, mc.Flag);
			}
		}
	}
}

void SSPhysics::Shutdown() {
}

