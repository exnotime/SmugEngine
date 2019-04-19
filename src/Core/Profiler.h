#pragma once
#include "Timer.h"
#include <EASTL/string.h>
#include <EASTL/vector.h>
namespace smug {
	class Profiler {
	public:
		Profiler();
		~Profiler();
		void Init();
		void NewFrame();
		void Stamp(const char* name);
		void Print();
	private:
		Timer m_Timer;
		struct Entry { eastl::string name; double time; };
		eastl::vector<Entry> m_Entries;
	};
}