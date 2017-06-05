#pragma once
#include "SubSystem.h"
#include <stdint.h>
#include <vector>
#define SUBSYSTEM_INPUT_ORDER -1
struct SubSystemEntry {
	uint32_t start;
	uint32_t update;
	uint32_t shutdown;
	SubSystem* ss;
};

class SubSystemSet {
  public:
	SubSystemSet();
	~SubSystemSet();

	void AddSubSystem( SubSystem* ss, uint32_t startUpPrio = -1, uint32_t updatePrio = -1, uint32_t shutdownPrio = -1);
	void StartSubSystems();
	void UpdateSubSystems(const double deltaTime);
	void ShutdownSubSystems();
	void Clear();
  private:
	bool m_Updated = false;
	std::vector<SubSystemEntry> m_Entries;
	uint32_t m_Enumerator = 0;
};