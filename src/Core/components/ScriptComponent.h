#pragma once
#include <Core/script/ScriptEngine.h>
namespace smug {
/*
A script file can contain multiple functions
At load we check if it contains several pre-defined functions and store them in a bitmask
*/
enum FunctionBits {
	SCRIPT_FUNC_UPDATE = 0x1, //called at update every frame
	SCRIPT_FUNC_RENDER = 0x2, //called before render every frame
	SCRIPT_FUNC_LOAD = 0x4,   //called at load time
	SCRIPT_FUNC_INIT = 0x8    //called after load
};

struct ScriptComponent {
	AngelScript::asIScriptModule* Module = nullptr;
	uint32_t FunctionBitMask = 0;
	static unsigned int Flag;
};

static void CreateScriptComponent(const char* filename, ScriptComponent& sc) {
	sc.Module = g_ScriptEngine.CompileScriptToModule(filename);
	if (sc.Module->GetFunctionByDecl("void Update(float)")) {
		sc.FunctionBitMask |= SCRIPT_FUNC_UPDATE;
	}
	if (sc.Module->GetFunctionByDecl("void Render()")) {
		sc.FunctionBitMask |= SCRIPT_FUNC_RENDER;
	}
	if (sc.Module->GetFunctionByDecl("void Load()")) {
		sc.FunctionBitMask |= SCRIPT_FUNC_LOAD;
	}
	if (sc.Module->GetFunctionByDecl("void Init()")) {
		sc.FunctionBitMask |= SCRIPT_FUNC_INIT;
	}
}
}
