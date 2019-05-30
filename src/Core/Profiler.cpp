#include "Profiler.h"
#include <Imgui/imgui.h>
using namespace smug;

Profiler::Profiler() {

}
Profiler::~Profiler() {

}
void Profiler::Init(){
	m_Timer.Reset();
}

void Profiler::NewFrame(){
	m_Entries.clear();
	Entry e = { "Start", m_Timer.Reset() };
	m_Entries.push_back(e);
}

void Profiler::Stamp(const char* name) {
	Entry e = { name, m_Timer.Tick() };
	m_Entries.push_back(e);
}

void Profiler::Print() {
	if (m_Entries.empty())
		return;
	ImGui::Begin("Profiler");
	uint32_t entryCount = m_Entries.size();
	for (uint32_t i = 1; i < entryCount - 1; ++i) {
		ImGui::Text("%s : %f ms", m_Entries[i].name, m_Entries[i + 1].time * 1000.0);
	}
	ImGui::End();
}