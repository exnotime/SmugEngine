#include "Engine.h"
#include "SubSystem/SSToken.h"

using namespace smug;
using namespace engine;

SSToken m_SSToken;

void LoadSubSystems(SubSystemSet& SSSet) {
	SSSet.AddSubSystem(&m_SSToken);
}

void ENGINE_DLL smug::engine::LoadSubSystems(SubSystemSet & SSSet)
{
	return void ENGINE_DLL();
}
