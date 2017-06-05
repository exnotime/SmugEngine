#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <Imgui/imgui.h>
#include <Imgui/imgui_impl_glfw_vulkan.h>
#define g_Input Input::GetInstance()

struct Gamepad {
	float LeftX;
	float LeftY;
	float RightX;
	float RightY;
	float LeftTrigger;
	float RightTrigger;
	bool A;
	bool B;
	bool X;
	bool Y;
	bool LB;
	bool RB;
	bool Start;
	bool Select;
	bool LeftClick;
	bool RightClick;
	bool DpadLeft;
	bool DpadRight;
	bool DpadUp;
	bool DpadDown;
	bool Connected;
};
class Input {
  public:
	~Input();
	static Input& GetInstance();
	void SetKeyState(int key, int state);
	void SetMousebuttonState(int button, int state);
	void SetGamepadConnected(int pid, bool status);
	bool IsMousebuttonDown(int button);
	bool IsMousebuttonPushed(int button);
	bool IsKeyDown(int key);
	bool IsKeyPushed(int key);
	bool IsGamepadConnected(int pid);
	void SetCursorMode(GLFWwindow* window,int mode);
	void SetMousePos(double x, double y);
	Gamepad& GetGamepadState(int pid);
	Gamepad& GetGamepadStateLastFrameState(int pid);
	glm::dvec2 GetMouseDelta();
	glm::dvec2 GetMousePos();
	void Update();
	void SetMouseDeltaUpdate(bool val);
  private:
	Input();
	int m_Keys[GLFW_KEY_LAST];
	int m_KeysLastFrame[GLFW_KEY_LAST];
	int m_MouseButtons[GLFW_MOUSE_BUTTON_LAST];
	int m_MouseButtonsLastFrame[GLFW_MOUSE_BUTTON_LAST];
	Gamepad m_GamePads[4];
	Gamepad m_GamePadLastFrame[4];
	glm::dvec2 m_MousePos;
	glm::dvec2 m_MouseDelta;
	float m_MouseScroll;
	bool m_UpdateMouseDelta;
};
static void MousePosCallback(GLFWwindow* window, double xpos, double ypos) {
	g_Input.SetMousePos(xpos, ypos);
}
static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	g_Input.SetMousebuttonState(button, action);
	ImGui_ImplGlfwVulkan_MouseButtonCallback(window, button, action, mods);
}
static void KeyboardCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	g_Input.SetKeyState(key, action);
	ImGui_ImplGlfwVulkan_KeyCallback(window, key, scancode, action, mods);
}
static void CharCallback(GLFWwindow* window, unsigned int c) {
	ImGui_ImplGlfwVulkan_CharCallback(window, c);
};