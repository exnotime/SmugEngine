#pragma once
#include "Window.h"

class GraphicsEngine;

class Engine {
public:
	Engine();
	~Engine();
	void Init();
	void Run();
private:
	Window* m_Window;
	GraphicsEngine* m_vkGFX;
};