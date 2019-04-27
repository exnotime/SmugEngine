#pragma once
#include <stdint.h>
#include "../AssetExport.h"
namespace smug {
	//angelscript interface for building rendercommands
	namespace if_shader {
		struct ScriptPipelineState;

		ScriptPipelineState* GetPSO(uint32_t hash);
		ASSET_DLL void InitInterface();
	}
}