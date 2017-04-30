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
#include "datasystem/ComponentManager.h"
#include "subsystem/SubSystemSet.h"
#include "subsystem/systems/SSCamera.h"
#include "subsystem/systems/SSRender.h"
#include "GlobalSystems.h"
#include "Timer.h"
#include "AssetLoader/AssetLoader.h" //temp

Engine::Engine() {

}

Engine::~Engine() {
	delete m_Window;
	m_MainSubSystemSet->Clear();
	delete m_MainSubSystemSet;
	delete m_GlobalTimer;
	globals::Clear();
	glfwTerminate();
}

void Engine::Init() {
	//set up window
	m_Window = new Window();
	WindowSettings ws;
	ws.X = 50;
	ws.Y = 40;
	ws.Width = 1920;
	ws.Height = 1080;
	ws.HighDPI = false;
	ws.OpenGL = true;
	ws.Title = "Tephra";
	ws.Vsync = true;
	ws.BorderLess = false;
	m_Window->Initialize(ws);

	glfwSetKeyCallback(m_Window->GetWindow(), KeyboardCallBack);
	glfwSetMouseButtonCallback(m_Window->GetWindow(), MouseButtonCallback);
	glfwSetCursorPosCallback(m_Window->GetWindow(), MousePosCallback);
	g_Input.SetCursorMode(m_Window->GetWindow(), GLFW_CURSOR_DISABLED);

	//set up graphics engine
	globals::g_Gfx = new GraphicsEngine();
	HWND hWnd = glfwGetWin32Window(m_Window->GetWindow());
	globals::g_Gfx->Init(glm::vec2(ws.Width, ws.Height), ws.Vsync, hWnd);

	//create component buffers
	g_ComponentManager.AddComponentType(100, sizeof(TransformComponent), TransformComponent::Flag, "TransformComponent");
	g_ComponentManager.AddComponentType(100, sizeof(ModelComponent), ModelComponent::Flag, "ModelComponent");
	g_ComponentManager.AddComponentType(100, sizeof(RigidBodyComponent), RigidBodyComponent::Flag, "RigidBodyComponent");
	g_ComponentManager.AddComponentType(10, sizeof(CameraComponent), CameraComponent::Flag, "CameraComponent");

	m_MainSubSystemSet = new SubSystemSet();
	m_MainSubSystemSet->AddSubSystem(new SSCamera());
	m_MainSubSystemSet->AddSubSystem(new SSRender());
	m_MainSubSystemSet->StartSubSystems();

	m_GlobalTimer = new Timer();
	m_GlobalTimer->Reset();
	g_AssetLoader.LoadAsset("assets/KoopaTroopa/Koopa.dae");
	globals::g_Gfx->TransferToGPU();
}

void Engine::Run() {
	int mode = GLFW_CURSOR_DISABLED;
	while (!glfwWindowShouldClose(m_Window->GetWindow())) {

		if (g_Input.IsKeyPushed(GLFW_KEY_L)) {
			mode = (mode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
			g_Input.SetCursorMode(m_Window->GetWindow(), mode);
		}

		if (g_Input.IsKeyDown(GLFW_KEY_ESCAPE)) {
			break;
		}

		m_MainSubSystemSet->UpdateSubSystems(m_GlobalTimer->Tick());

		globals::g_Gfx->Render();
		globals::g_Gfx->Swap();

		g_Input.Update();
		glfwPollEvents();
	}

}