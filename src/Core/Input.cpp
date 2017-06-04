#include "Input.h"


Input::Input() {
	m_MousePos = glm::vec2(0);
	m_UpdateMouseDelta = true;
}
Input::~Input() {

}

Input& Input::GetInstance() {
	static Input instance;
	return instance;
}

void Input::Update() {
	m_MouseDelta = glm::dvec2(0);
	memcpy(m_KeysLastFrame, m_Keys, sizeof(int) * GLFW_KEY_LAST);
	memcpy(m_MouseButtonsLastFrame, m_MouseButtons, sizeof(int) * GLFW_MOUSE_BUTTON_LAST);
	memcpy(m_GamePadLastFrame, m_GamePads, sizeof(Gamepad) * 4);
	//update gamepads
	for (int i = 0; i < 4; i++) {
		m_GamePads[i].Connected = (bool)glfwJoystickPresent(i);
		if (m_GamePads[i].Connected) {
			const char* name = glfwGetJoystickName(i);
			//update axes
			int c = 0;
			const float* axes = glfwGetJoystickAxes(i, &c);
			const unsigned char* buttons = glfwGetJoystickButtons(i, &c);
			m_GamePads[i].LeftX = axes[0];
			m_GamePads[i].LeftY = axes[1];
			m_GamePads[i].RightX = axes[2];
			m_GamePads[i].RightY = axes[3];
			m_GamePads[i].LeftTrigger = axes[4] * 0.5f + 0.5f; //modify to 0 -> 1 instead of -1 -> 1
			m_GamePads[i].RightTrigger = axes[5] * 0.5f + 0.5f;
			
			m_GamePads[i].A = buttons[0];
			m_GamePads[i].B = buttons[1];
			m_GamePads[i].X = buttons[2];
			m_GamePads[i].Y = buttons[3];
			m_GamePads[i].LB = buttons[4];
			m_GamePads[i].RB = buttons[5];
			m_GamePads[i].Select = buttons[6];
			m_GamePads[i].Start = buttons[7];
			m_GamePads[i].LeftClick = buttons[8];
			m_GamePads[i].RightClick = buttons[9];
			m_GamePads[i].DpadUp = buttons[10];
			m_GamePads[i].DpadRight = buttons[11];
			m_GamePads[i].DpadDown = buttons[12];
			m_GamePads[i].DpadLeft = buttons[13];
		}
	}

}

void Input::SetKeyState(int key, int state){
	m_Keys[key] = state;
}

void Input::SetGamepadConnected(int pid, bool status) {
	m_GamePads[pid].Connected = status;
}

bool Input::IsKeyDown(int key){
	return m_Keys[key] == GLFW_PRESS || m_Keys[key] == GLFW_REPEAT;
}

bool Input::IsKeyPushed(int key) {
	return (m_KeysLastFrame[key] == GLFW_PRESS || m_KeysLastFrame[key] == GLFW_REPEAT) && m_Keys[key] == GLFW_RELEASE;
}

void Input::SetMousebuttonState(int button, int state) {
	m_MouseButtons[button] = state;
}
bool Input::IsMousebuttonDown(int button) {
	return m_MouseButtons[button] == GLFW_PRESS;
}

bool Input::IsMousebuttonPushed(int button) {
	return (m_MouseButtonsLastFrame[button] == GLFW_PRESS || m_MouseButtonsLastFrame[button] == GLFW_REPEAT) && m_MouseButtons[button] == GLFW_RELEASE;
}

bool Input::IsGamepadConnected(int pid) {
	return m_GamePads[pid].Connected;
}
void Input::SetCursorMode(GLFWwindow* window, int mode) {
	glfwSetInputMode(window, GLFW_CURSOR, mode);
	m_MouseDelta = glm::dvec2(0);
}
void Input::SetMousePos(double x, double y) {
	if(m_UpdateMouseDelta)
		m_MouseDelta = glm::dvec2(x, y) - m_MousePos;
	m_MousePos = glm::dvec2(x, y);
}

Gamepad& Input::GetGamepadState(int pid) {
	return m_GamePads[pid];
}
Gamepad& Input::GetGamepadStateLastFrameState(int pid) {
	return m_GamePadLastFrame[pid];
}

glm::dvec2 Input::GetMouseDelta() {
	return m_MouseDelta;
}
glm::dvec2 Input::GetMousePos() {
	return m_MousePos;
}

void Input::SetMouseDeltaUpdate(bool val){
	m_UpdateMouseDelta = val;
}