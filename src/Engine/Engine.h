#pragma once
#include <Core/subsystem/SubSystemSet.h>
#include "EngineExport.h"
namespace smug {
	namespace engine {
		ENGINE_DLL void LoadSubSystems(SubSystemSet& SSSet);
		void ENGINE_DLL LoadScriptInterface();
		void ENGINE_DLL Shutdown();
	}
}