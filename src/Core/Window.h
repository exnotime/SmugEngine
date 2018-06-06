#pragma once
#include <string>
struct GLFWwindow;
namespace smug {

struct WindowSettings {
	int Width = 1600;
	int Height = 900;
	int X = -1;
	int Y = -1;
	bool Fullscreen = false;
	bool Resizeable = false;
	bool HighDPI = false;
	bool BorderLess = false;
	bool Vsync = false;
	bool OpenGL = true;
	std::string Title = "Untitled window";
};

class Window {
  public:
	Window();
	~Window();
	void Initialize(const WindowSettings& windowSettings);

	GLFWwindow* GetWindow() const;
	const WindowSettings& GetWindowSettings() const;
	void MakeCurrent();

  private:
	GLFWwindow* m_Window = nullptr;
	void* m_GLContext;
	WindowSettings m_WindowSettings;
	bool m_Initialized = false;
};
}
