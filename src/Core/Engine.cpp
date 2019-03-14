#include "engine.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
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
#include "subsystem/systems/SSLevel.h"
#include "subsystem/systems/SSLevel.h"
#include "GlobalSystems.h"
#include "Timer.h"
#include "if_Assets.h"
#include "if_Render.h"
#include "entity/if_Entity.h"
#include "components/if_Components.h"
#include "script/ScriptEngine.h"

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
	globals::g_Components->AddComponentType(100, sizeof(TransformComponent), TransformComponent::Flag, "TransformComponent");
	globals::g_Components->AddComponentType(100, sizeof(ModelComponent), ModelComponent::Flag, "ModelComponent");
	globals::g_Components->AddComponentType(100, sizeof(RigidBodyComponent), RigidBodyComponent::Flag, "RigidBodyComponent");
	globals::g_Components->AddComponentType(100, sizeof(ScriptComponent), ScriptComponent::Flag, "ScriptComponent");
	globals::g_Components->AddComponentType(3, sizeof(CameraComponent), CameraComponent::Flag, "CameraComponent");


	AngelScript::asIScriptModule* mod = g_ScriptEngine.CompileScriptToModule("script/LoadingTest.as");
	g_ScriptEngine.ExecuteModule(mod, "void Load()");

	m_MainSubSystemSet = new SubSystemSet();
	m_MainSubSystemSet->AddSubSystem(new SSCamera());
	m_MainSubSystemSet->AddSubSystem(new SSLevel());
	m_MainSubSystemSet->AddSubSystem(new SSPhysics());
	m_MainSubSystemSet->AddSubSystem(new SSRender());
	m_MainSubSystemSet->StartSubSystems();

	m_GlobalTimer = new Timer();
	m_GlobalTimer->Reset();

	m_ProfilerTimer = new Timer();

	m_ImguiCtx = ImGui::CreateContext();
	ImGui_ImplGlfwVulkan_Init_Data* imguiData = globals::g_Gfx->GetImguiInit();
	ImGui_ImplGlfwVulkan_Init(m_Window->GetWindow(), false, imguiData);
	globals::g_Gfx->CreateImguiFont(m_ImguiCtx);
	delete imguiData;
	//assets need to be loaded before this
	globals::g_Gfx->TransferToGPU();

	//test script writer
	//ScriptWriter sw;
	//sw.AddVariable("test", VAR_INT);
	//sw.AddVariable("test2", VAR_FLOAT);
	//float test = 3.14f;
	//sw.AddVariable("test_with_data", VAR_FLOAT, &test);
	//sw.AddVariable("test3", VAR_BOOL);
	//int* array_test = (int*)malloc(sizeof(int) * 100);
	//for (int i = 0; i < 100; ++i) {
	//	array_test[i] = i;
	//}
	//sw.AddVariableArray("array_test", VAR_INT, 100, array_test);
	//sw.OpenFunction("TestFunction");
	//sw.AddSnippet("print(\"Test printing from a generated script file\\n\")");
	//sw.CloseFunction();
	//sw.WriteToFile("script/TestScript.ss");
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
		globals::g_Gfx->PrintStats();
		m_MainSubSystemSet->UpdateSubSystems(tick);
		globals::g_Physics->Update(tick);
		globals::g_Gfx->Render();
		globals::g_Gfx->Swap();

		char buffer[400];
		sprintf(buffer, "Smug Engine : FPS = %f", 1.0 / tick);

		glfwSetWindowTitle(m_Window->GetWindow(), buffer);
		g_Input.Update();
		glfwPollEvents();
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
	delete m_ProfilerTimer;
	globals::Clear();
	glfwTerminate();

}