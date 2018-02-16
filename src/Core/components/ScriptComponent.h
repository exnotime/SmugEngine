#pragma once
#include <Core/script/ScriptEngine.h>
namespace smug {
	/*
	A script file can contain multiple functions
	At load we check if it contains several pre-defined functions and store them in a bitmask
	*/
	enum FunctionBits {
		UPDATE = 0x1, //called at update every frame
		RENDER = 0x2, //called before render every frame
		LOAD = 0x4,   //called at load time
		INIT = 0x8    //called after load
	};

	struct ScriptComponent {
		AngelScript::asIScriptModule* Module = nullptr;
		uint32_t FunctionBitMask = 0;
		static unsigned int Flag;
	};

	static void CreateScriptComponent(const char* filename, ScriptComponent& sc) {
		sc.Module = g_ScriptEngine.CompileScriptToModule(filename);
		if (sc.Module->GetFunctionByDecl("void Update(float)")) {
			sc.FunctionBitMask |= UPDATE;
		}
		if (sc.Module->GetFunctionByDecl("void Render()")) {
			sc.FunctionBitMask |= RENDER;
		}
		if (sc.Module->GetFunctionByDecl("void Load()")) {
			sc.FunctionBitMask |= LOAD;
		}
		if (sc.Module->GetFunctionByDecl("void Init()")) {
			sc.FunctionBitMask |= INIT;
		}
	}
}
