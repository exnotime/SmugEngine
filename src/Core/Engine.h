#pragma once
class Window;
class GraphicsEngine;
class SubSystemSet;

class Engine {
public:
	Engine();
	~Engine();
	void Init();
	void Run();
private:
	Window* m_Window;
	GraphicsEngine* m_vkGFX;
	SubSystemSet* m_MainSubSystemSet;
};