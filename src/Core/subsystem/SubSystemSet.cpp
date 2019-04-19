#include "SubSystemSet.h"
#include <algorithm>
#include "../Profiler.h"

using namespace smug;
SubSystemSet::SubSystemSet() {

}

SubSystemSet::~SubSystemSet() {
}

void SubSystemSet::AddSubSystem( SubSystem* ss, const char* debugName, uint32_t startUpPrio, uint32_t updatePrio, uint32_t shutdownPrio) {
	SubSystemEntry sse;
	sse.ss = ss;

	sse.start = (startUpPrio == SUBSYSTEM_INPUT_ORDER) ? (uint32_t)m_Entries.size() : startUpPrio;
	sse.update = (updatePrio == SUBSYSTEM_INPUT_ORDER) ? (uint32_t)m_Entries.size() : updatePrio;
	sse.shutdown = (shutdownPrio == SUBSYSTEM_INPUT_ORDER) ? (uint32_t)m_Entries.size() : shutdownPrio;
#if defined(_DEBUG)
	if(debugName)
		sse.debug_name = debugName;
#endif
	ss->SetID(++m_Enumerator);
	m_Entries.push_back(sse);
	m_Updated = true;
}

void SubSystemSet::StartSubSystems() {
	auto sortByStartUp = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
		return lhs.start < rhs.start;
	};
	std::sort(m_Entries.begin(), m_Entries.end(), sortByStartUp);

	for (auto& system : m_Entries) {
		system.ss->Startup();
	}
}

void SubSystemSet::UpdateSubSystems(const double deltaTime, Profiler* prof) {
	if (m_Updated) {
		auto sortByUpdate = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
			return lhs.update < rhs.update;
		};
		std::sort(m_Entries.begin(), m_Entries.end(), sortByUpdate);
		m_Updated = false;
	}
#if defined(_DEBUG)
	prof->Stamp("SubSystemSet");
	for (auto& system : m_Entries) {
		system.ss->Update(deltaTime);
		prof->Stamp(system.debug_name.c_str());
	}
#else
	for (auto& system : m_Entries) {
		system.ss->Update(deltaTime);
	}
#endif
}

void SubSystemSet::ShutdownSubSystems() {
	auto sortByShutdown = [](const SubSystemEntry& lhs, const SubSystemEntry& rhs) {
		return lhs.shutdown < rhs.shutdown;
	};
	std::sort(m_Entries.begin(), m_Entries.end(), sortByShutdown);
	
	for (auto& system : m_Entries) {
		system.ss->Shutdown();
	}
}

void SubSystemSet::Clear() {
	ShutdownSubSystems();

	for (auto& system : m_Entries) {
		delete system.ss;
	}
	m_Entries.clear();
}