#pragma once
#include "SubSystem.h"
#include <stdint.h>
#include <EASTL/vector.h>
#include <EASTL/string.h>
#include "../Timer.h"
#define SUBSYSTEM_INPUT_ORDER -1
namespace smug {
	class Profiler;

struct SubSystemEntry {
	uint32_t start;
	uint32_t update;
	uint32_t shutdown;
	SubSystem* ss;
#if defined(_DEBUG)
	eastl::string debug_name;
#endif
};

class SubSystemSet {
  public:
	SubSystemSet();
	~SubSystemSet();

	void AddSubSystem(SubSystem* ss, const char* debug_name = nullptr, uint32_t startUpPrio = -1, uint32_t updatePrio = -1, uint32_t shutdownPrio = -1);
	void StartSubSystems();
	void UpdateSubSystems(const double deltaTime, Profiler* prof);
	void ShutdownSubSystems();
	void Clear();
  private:
	bool m_Updated = false;
	eastl::vector<SubSystemEntry> m_Entries;
	uint32_t m_Enumerator = 0;
};
}