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
	Entity& e = g_EntityManager.CreateEntity();

	TransformComponent tc;
	tc.Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	tc.Position = glm::vec3(0.0f, 0.08f, 0.0f);
	tc.Scale = glm::vec3(10.0f);
	tc.Transform = glm::mat4(1.0f);
	g_ComponentManager.CreateComponent(&tc, e, tc.Flag);

	ModelComponent mc;
	mc.ModelHandle = g_AssetLoader.LoadAsset("assets/KoopaTroopa/koopa.obj");
	mc.Tint = glm::vec4(1.0f);
	g_ComponentManager.CreateComponent(&mc, e, mc.Flag);
}

void SSRender::Update(const double deltaTime) {

	int flag = ModelComponent::Flag | TransformComponent::Flag;
	RenderQueue* rq = globals::g_Gfx->GetRenderQueue();
	//Test Imgui
	ImGui::ShowTestWindow();
	ImGui::Begin("Enities");
	if (ImGui::TreeNode("EntityList")) {
		for (auto& e : g_EntityManager.GetEntityList()) {
			TransformComponent* tc = (TransformComponent*)g_ComponentManager.GetComponent(e, TransformComponent::Flag);
			if (ImGui::TreeNode((void*)(intptr_t)e.UID, "Entity %d", e.UID)) {
				ImGui::Text("Position: x=%f y=%f z=%f", tc->Position.x, tc->Position.y, tc->Position.z);
				ImGui::Text("Rotation: x=%f y=%f z=%f w=%f", tc->Orientation.x, tc->Orientation.y, tc->Orientation.z, tc->Orientation.w);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();
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
	
	
}

void SSRender::Shutdown() {
}

