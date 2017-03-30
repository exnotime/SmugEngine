#include "engine.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glm/glm.hpp>
#include "Input.h"
#include "Window.h"
#include "Graphics/GraphicsEngine.h"
#include "components/CameraComponent.h"
#include "components/TransformComponent.h"
#include "datasystem/ComponentManager.h"
#include "subsystem/SubSystemSet.h"
#include "subsystem/systems/SSCamera.h"

Engine::Engine() {

}

Engine::~Engine() {
	delete m_Window;
	delete m_vkGFX;
	m_MainSubSystemSet->Clear();
	delete m_MainSubSystemSet;
	glfwTerminate();
}

void Engine::Init() {
	//set up window
	m_Window = new Window();
	WindowSettings ws;
	ws.X = 50;
	ws.Y = 40;
	ws.Width = 1600;
	ws.Height = 900;
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
	m_vkGFX = new GraphicsEngine();
	HWND hWnd = glfwGetWin32Window(m_Window->GetWindow());
	m_vkGFX->Init(glm::vec2(ws.Width, ws.Height), ws.Vsync, hWnd);
	//create component buffers
	g_ComponentManager.AddComponentType(1000, sizeof(TransformComponent), TransformComponent::Flag, "TransformComponent");
	g_ComponentManager.AddComponentType(10, sizeof(CameraComponent), CameraComponent::Flag, "CameraComponent");

	m_MainSubSystemSet = new SubSystemSet();
	m_MainSubSystemSet->AddSubSystem(new SSCamera());

}

void Engine::Run() {
	int mode = GLFW_CURSOR_DISABLED;
	while (!glfwWindowShouldClose(m_Window->GetWindow())) {
		if (g_Input.IsKeyPushed(GLFW_KEY_L)) {
			mode = (mode == GLFW_CURSOR_NORMAL) ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL;
			g_Input.SetCursorMode(m_Window->GetWindow(), mode);
		}
		if (g_Input.IsKeyPushed(GLFW_KEY_RIGHT)) {
			m_vkGFX->NextPipeline();
		}
		if (g_Input.IsKeyPushed(GLFW_KEY_LEFT)) {
			m_vkGFX->PrevPipeline();
		}
		m_vkGFX->Render();
		m_vkGFX->Swap();

		if (g_Input.IsKeyDown(GLFW_KEY_ESCAPE))
			break;

		g_Input.Update();
		glfwPollEvents();
	}

}