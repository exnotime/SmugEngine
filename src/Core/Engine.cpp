#include "engine.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include <Graphics/GraphicsEngine.h>
#include "entity/EntityManager.h"
#include "Input.h"
#include "Window.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "components/ModelComponent.h"
#include "components/RigidBodyComponent.h"
#include "components/ScriptComponent.h"
#include "datasystem/ComponentManager.h"
#include "subsystem/SubSystemSet.h"
#include "subsystem/systems/SSCamera.h"
#include "subsystem/systems/SSRender.h"
#include "subsystem/systems/SSPhysics.h"
#include "GlobalSystems.h"
#include "Timer.h"
#include "if_Assets.h"
#include "if_Render.h"
#include "entity/if_Entity.h"
#include "components/if_Components.h"
#include "script/ScriptEngine.h"
#include "Profiler.h"

#include <Utility/ScriptWriter.h>
using namespace smug;
Engine::Engine() {

}

Engine::~Engine() {
}

void Engine::Init() {
	//set up window
	m_Window = new Window();
	WindowSettings ws;
	ws.X = 100;
	ws.Y = 100;
	ws.Width = 1920;
	ws.Height = 1080;
	ws.HighDPI = false;
	ws.OpenGL = true;
	ws.Title = "Smug Engine";
	ws.Vsync = true;
	ws.BorderLess = false;
	m_Window->Initialize(ws);

	glfwSetKeyCallback(m_Window->GetWindow(), KeyboardCallBack);
	glfwSetMouseButtonCallback(m_Window->GetWindow(), MouseButtonCallback);
	glfwSetCursorPosCallback(m_Window->GetWindow(), MousePosCallback);
	glfwSetCharCallback(m_Window->GetWindow(), CharCallback);
	glfwSetScrollCallback(m_Window->GetWindow(), ScrollCallback);
	g_Input.SetCursorMode(m_Window->GetWindow(), GLFW_CURSOR_DISABLED);

	g_AssetLoader.Init("data", false);
	g_AssetLoader.LoadStringPool("Assets.strings");

	//set up graphics engine
	uint32_t s = sizeof(GraphicsEngine);
	globals::g_Gfx = new GraphicsEngine();
	HWND hWnd = glfwGetWin32Window(m_Window->GetWindow());
	globals::g_Gfx->Init(glm::vec2(ws.Width, ws.Height), ws.Vsync, hWnd);

	//set up physics engine
	globals::g_Physics = new PhysicsEngine();
	globals::g_Physics->Init();

	g_ScriptEngine.Init();

	if_asset::RegisterInterface();
	if_entity::RegisterEntityInterface();
	if_component::InitComponentInterface();
	if_render::InitInterface();
	//set up entity manager
	globals::g_EntityManager = new EntityManager();

	//create component buffers
	globals::g_Components = new ComponentManager();
	globals::g_Components->AddComponentType(1000, sizeof(TransformComponent), TransformComponent::Flag, "TransformComponent");
	globals::g_Components->AddComponentType(1000, sizeof(ModelComponent), ModelComponent::Flag, "ModelComponent");
	globals::g_Components->AddComponentType(1000, sizeof(RigidBodyComponent), RigidBodyComponent::Flag, "RigidBodyComponent");
	globals::g_Components->AddComponentType(1000, sizeof(ScriptComponent), ScriptComponent::Flag, "ScriptComponent");
	globals::g_Components->AddComponentType(3, sizeof(CameraComponent), CameraComponent::Flag, "CameraComponent");


	AngelScript::asIScriptModule* mod = g_ScriptEngine.CompileScriptToModule("script/LoadingTest.as");
	g_ScriptEngine.ExecuteModule(mod, "void Load()");

	m_MainSubSystemSet = new SubSystemSet();
	m_MainSubSystemSet->AddSubSystem(new SSCamera(), "SSCamera");
	m_MainSubSystemSet->AddSubSystem(new SSPhysics(), "SSPhysics");
	m_MainSubSystemSet->AddSubSystem(new SSRender(), "SSRender");
	m_MainSubSystemSet->StartSubSystems();

	m_GlobalTimer = new Timer();
	m_GlobalTimer->Reset();
	m_Profiler = new Profiler();

	m_ImguiCtx = ImGui::CreateContext();
	ImGui_ImplGlfwVulkan_Init_Data* imguiData = globals::g_Gfx->GetImguiInit();
	ImGui_ImplGlfwVulkan_Init(m_Window->GetWindow(), false, imguiData);
	globals::g_Gfx->CreateImguiFont(m_ImguiCtx);
	delete imguiData;
	//assets need to be loaded before this
	globals::g_Gfx->TransferToGPU();
}

void Engine::Run() {
	
	int mode = GLFW_CURSOR_DISABLED;
	while (!glfwWindowShouldClose(m_Window->GetWindow())) {
		double tick = m_GlobalTimer->Tick();
		ImGui_ImplGlfwVulkan_NewFrame();

		if (g_Input.IsKeyPushed(GLFW_KEY_F3)) {
			mode = (mode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
			g_Input.SetCursorMode(m_Window->GetWindow(), mode);
			g_Input.SetMouseDeltaUpdate(mode != GLFW_CURSOR_NORMAL);
			g_Input.SetEnableInput(mode != GLFW_CURSOR_NORMAL);
		}
		if (mode == GLFW_CURSOR_DISABLED) {
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2(0, 0);
			io.MouseDelta = ImVec2(0, 0);
			memset(&io.MouseDown, 0x0, sizeof(io.MouseDown));
			memset(&io.MouseClicked, 0x0, sizeof(io.MouseClicked));
		}

		if (g_Input.IsKeyDown(GLFW_KEY_ESCAPE)) {
			break;
		}
		m_Profiler->Print();
		m_Profiler->NewFrame();
		m_MainSubSystemSet->UpdateSubSystems(tick, m_Profiler);
		globals::g_Gfx->Render();
		m_Profiler->Stamp("GfxRender");
		globals::g_Gfx->Swap();
		m_Profiler->Stamp("Swap");
		g_Input.Update();
		m_Profiler->Stamp("UpdateInput");
		glfwPollEvents();
		m_Profiler->Stamp("PollEvents");
		char buffer[64];
		sprintf(buffer, "Smug Engine : FPS = %f", 1.0 / tick);
		glfwSetWindowTitle(m_Window->GetWindow(), buffer);

	}

}

void Engine::Shutdown() {
	g_AssetLoader.Close();
	ImGui_ImplGlfwVulkan_Shutdown();
	ImGui::DestroyContext(m_ImguiCtx);
	delete m_Window;
	m_MainSubSystemSet->Clear();
	delete m_MainSubSystemSet;
	delete m_GlobalTimer;
	delete m_Profiler;
	globals::Clear();
	glfwTerminate();

}