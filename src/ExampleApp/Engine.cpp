#include "engine.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Input.h"
#include "GraphicsEngine.h"

Engine::Engine() {

}

Engine::~Engine() {
	delete m_Window;
	delete m_vkGFX;
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
	ws.Title = "NaiveEngine";
	ws.Vsync = true;
	ws.BorderLess = false;
	m_Window->Initialize(ws);

	glfwSetKeyCallback(m_Window->GetWindow(), KeyboardCallBack);
	glfwSetMouseButtonCallback(m_Window->GetWindow(), MouseButtonCallback);
	glfwSetCursorPosCallback(m_Window->GetWindow(), MousePosCallback);
	g_Input.SetCursorMode(m_Window->GetWindow(), GLFW_CURSOR_DISABLED);

	m_vkGFX = new GraphicsEngine();
	m_vkGFX->Init(m_Window->GetWindow());
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