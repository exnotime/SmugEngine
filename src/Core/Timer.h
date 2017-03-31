#pragma once
#include <stdint.h>
class Timer {
public:
	Timer();
	~Timer();
	double Tick();
	double Reset();

private:
	uint64_t m_Start;
	uint64_t m_LastTick;
	uint64_t m_TimerFreq;
};