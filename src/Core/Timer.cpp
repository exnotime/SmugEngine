#include "Timer.h"
#include <GLFW/glfw3.h>
Timer::Timer() {
	m_Start = glfwGetTimerValue();
	m_TimerFreq = glfwGetTimerFrequency();
	m_LastTick = 0;
}

Timer::~Timer() {

}

double Timer::Tick() {
	uint64_t time = glfwGetTimerValue();
	uint64_t t = time - m_LastTick;
	m_LastTick = time;
	return t / (double)m_TimerFreq;
}

double Timer::Reset() {
	uint64_t time = glfwGetTimerValue();
	uint64_t t = time - m_Start;
	m_Start = time;
	m_LastTick = time;
	return t / (double)m_TimerFreq;
}