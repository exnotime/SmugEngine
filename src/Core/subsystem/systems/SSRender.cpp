#include "SSRender.h"
#include "Core/components/ModelComponent.h"
#include "Core/components/TransformComponent.h"
#include "Core/datasystem/ComponentManager.h"
#include "Core/entity/EntityManager.h"
#include <Graphics/GraphicsEngine.h>
#include <AssetLoader/AssetLoader.h>
#include <glm/gtx/transform.hpp>
#include "../../GlobalSystems.h"
#include <Imgui/imgui.h>

SSRender::SSRender() {

}

SSRender::~SSRender() {

}

void SSRender::Startup() {
	//spheres
	const int c = 16;
	const float d = 10;
	const float s = 5;
	ModelComponent mc;
	mc.ModelHandle = g_AssetLoader.LoadAsset("assets/cube/cube.obj");
	for (int z = -c; z < c; z++) {
		for (int y = -c; y < c; y++) {
			for (int x = -c; x < c; x++) {

				Entity& e = g_EntityManager.CreateEntity();

				TransformComponent tc;
				tc.Position = glm::vec3(x, y, z) * d;
				tc.Scale = glm::vec3(s);
				g_ComponentManager.CreateComponent(&tc, e, tc.Flag);
				mc.Tint = glm::vec4(x / float(c), 1.0f - (y / float(c)), 0.5f, 1.0f);
				g_ComponentManager.CreateComponent(&mc, e, mc.Flag);
			}
		}
	}
}

void SSRender::Update(const double deltaTime) {
	m_Timer.Reset();
	int flag = ModelComponent::Flag | TransformComponent::Flag;
	RenderQueue* rq = globals::g_Gfx->GetRenderQueue();
	//models
	for (auto& e : g_EntityManager.GetEntityList()) {
		if ((e.ComponentBitfield & flag) == flag) {
			ModelComponent* mc = (ModelComponent*)g_ComponentManager.GetComponent(e, ModelComponent::Flag);
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(e, TransformComponent::Flag);
			tc->Transform = glm::scale(tc->Scale) * glm::translate(tc->Position) * glm::toMat4(tc->Orientation);

			ShaderInput si;
			si.Transform = tc->Transform;
			si.Color = mc->Tint;
			rq->AddModel(mc->ModelHandle, si);
		}
	}
	float t = m_Timer.Reset();
	ImGui::Begin("Timing");
	ImGui::Text("SSRender: %f ms", t * 1000.0f);
	ImGui::End();

	ImGui::ShowMetricsWindow();
}

void SSRender::Shutdown() {
}

