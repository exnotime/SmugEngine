#include "SSRender.h"
#include "Core/components/ModelComponent.h"
#include "Core/components/TransformComponent.h"
#include "Core/components/RigidBodyComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"
#include <Graphics/GraphicsEngine.h>
#include <AssetLoader/AssetLoader.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../GlobalSystems.h"
#include <Imgui/imgui.h>

using namespace smug;
SSRender::SSRender() {

}

SSRender::~SSRender() {

}

float randf() {
	return float(rand()) / float(RAND_MAX);
}

void SSRender::Startup() {
	const int c = 3;
	const float d = 1;
	const float s = 1.0f;
	ModelComponent mc;
	mc.ModelHandle = g_AssetLoader.LoadAsset("assets/models/cube/cube.obj");
	//ResourceHandle shader = g_AssetLoader.LoadAsset("assets/shaders/filled.shader");
	//RenderQueue* rq = globals::g_Gfx->GetStaticQueue();

	for (int z = -c; z < c; z++) {
		for (int y = -c; y < c; y++) {
			for (int x = -c; x < c; x++) {

				Entity& e = globals::g_EntityManager->CreateEntity();

				TransformComponent tc;
				tc.Position = glm::vec3(x, y, z) * d;
				tc.Scale = glm::vec3(s);
				tc.Orientation = glm::quat();
				globals::g_Components->CreateComponent(&tc, e, tc.Flag);
				mc.Tint = glm::vec4(randf(), randf(), randf(), 1.0f);
				globals::g_Components->CreateComponent(&mc, e, mc.Flag);
				RigidBodyComponent rc;
				rc.Body = globals::g_Physics->CreateDynamicActor(tc.Position, tc.Orientation, tc.Scale / 2.0f, 1.0f, PHYSICS_SHAPE::CUBE);
				globals::g_Components->CreateComponent(&rc, e, rc.Flag);

				tc.Transform = glm::toMat4(tc.Orientation) * glm::scale(tc.Scale);
				tc.Transform[3][0] = tc.Position.x;
				tc.Transform[3][1] = tc.Position.y;
				tc.Transform[3][2] = tc.Position.z;

				//rq->AddModel(mc.ModelHandle, tc.Transform, mc.Tint);
			}
		}
	}
}

void SSRender::Update(const double deltaTime) {
	m_Timer.Reset();
	int flag = ModelComponent::Flag | TransformComponent::Flag;
	RenderQueue* rq = globals::g_Gfx->GetRenderQueue();
	//models
	auto& entities = globals::g_EntityManager->GetEntityList();
	uint32_t entityCount = (uint32_t)entities.size();

	static glm::vec4 tint = glm::vec4(1.0f);
	//ImGui::Begin("Scene");
	//ImGui::ColorEdit4("Global Color", &tint[0], false);
	//ImGui::End();

	for (uint32_t e = 0; e < entityCount; ++e) {
		auto& entity = entities[e];
		if ((entity.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)globals::g_Components->GetComponent(entity, ModelComponent::Flag);
			if (mc->Static || !mc->Visible) //it will already be on the static queue
				continue;

			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(entity, TransformComponent::Flag);
			tc->Transform = glm::toMat4(tc->Orientation) * glm::scale(tc->Scale);
			tc->Transform[3][0] = tc->Position.x;
			tc->Transform[3][1] = tc->Position.y;
			tc->Transform[3][2] = tc->Position.z;

			rq->AddModel(mc->ModelHandle, tc->Transform, mc->Tint);
		}
	}
	ImGui::Begin("Timing");
	double t = m_Timer.Reset() * 1000.0;
	ImGui::Text("SSRender: %f ms", t);
	ImGui::End();
}

void SSRender::Shutdown() {
}

