#pragma once
#include <AngelScript/ScriptEngine.h>
#include <AssetLoader/AssetLoader.h>
namespace smug {
	namespace if_asset {
		static ResourceHandle LoadModel(const eastl::string s) {
			return g_AssetLoader.LoadAsset(s.c_str());
		}

		static void RegisterInterface() {
			using namespace AngelScript;
			asIScriptEngine* engine = g_ScriptEngine.GetEngine();
			engine->RegisterGlobalFunction("uint64 LoadModel(string s)", asFUNCTION(LoadModel), asCALL_CDECL);
		}
	}
}