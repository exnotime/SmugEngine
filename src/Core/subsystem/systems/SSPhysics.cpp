#include "SSPhysics.h"
#include "../../GlobalSystems.h"
#include "Core/components/TransformComponent.h"
#include "Core/components/ModelComponent.h"
#include "Core/components/RigidBodyComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"
#include <Core/Input.h>
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
	for (auto& e : g_EntityManager.GetEntityList()) {
		if ((e.ComponentBitfield & flag) == flag) {
			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(e, TransformComponent::Flag);
			RigidBodyComponent* rc = (RigidBodyComponent*)globals::g_Components->GetComponent(e, RigidBodyComponent::Flag);

			tc->Position = rc->Body->Position;
			tc->Orientation = rc->Body->Orientation;
		}
	}
}

void SSPhysics::Shutdown() {
}

