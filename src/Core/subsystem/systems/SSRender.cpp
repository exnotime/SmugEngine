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
#include <Core/Profiler.h>

using namespace smug;
SSRender::SSRender() {

}

SSRender::~SSRender() {

}

float randf() {
	return float(rand()) / float(RAND_MAX);
}

void SSRender::Startup() {
	const int c = 2;
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
				rc.Body->UserData = e.UID;
				globals::g_Components->CreateComponent(&rc, e, rc.Flag);

				tc.Transform = glm::toMat4(tc.Orientation) * glm::scale(tc.Scale);
				tc.Transform[0][3] = tc.Position.x;
				tc.Transform[1][3] = tc.Position.y;
				tc.Transform[2][3] = tc.Position.z;

				//rq->AddModel(mc.ModelHandle, tc.Transform, mc.Tint);
			}
		}
	}
}

void SSRender::Update(const double deltaTime, Profiler* profiler) {
	int flag = ModelComponent::Flag | TransformComponent::Flag;
	RenderQueue* rq = globals::g_Gfx->GetRenderQueue();
	//models
	auto& entities = globals::g_EntityManager->GetEntityList();
	uint32_t entityCount = (uint32_t)entities.size();

	static glm::vec4 tint = glm::vec4(1.0f);

	for (uint32_t e = 0; e < entityCount; ++e) {
		auto& entity = entities[e];
		if ((entity.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)globals::g_Components->GetComponent(entity, ModelComponent::Flag);
			if (mc->Static || !mc->Visible) //it will already be on the static queue or not visible
				continue;

			TransformComponent* tc = (TransformComponent*)globals::g_Components->GetComponent(entity, TransformComponent::Flag);
			tc->Transform = glm::transpose(glm::toMat4(tc->Orientation));
			tc->Transform[0] *= tc->Scale.x;
			tc->Transform[1] *= tc->Scale.y;
			tc->Transform[2] *= tc->Scale.z;
			tc->Transform[0][3] = tc->Position.x;
			tc->Transform[1][3] = tc->Position.y;
			tc->Transform[2][3] = tc->Position.z;

			rq->AddModel(mc->ModelHandle, mc->Shader, tc->Transform, mc->Tint, mc->Layer);
		}
	}
}

void SSRender::Shutdown() {
}

