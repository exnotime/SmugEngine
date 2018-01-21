#pragma once
#include <Core/script/ScriptEngine.h>
#include <Core/GlobalSystems.h>
namespace if_entity{
	using namespace AngelScript;

	static uint32_t CreateEntity(uint32_t tag = 0) {
		Entity& e = globals::g_EntityManager->CreateEntity();
		e.Tag = tag;
		return e.UID;
	}

	static bool HasAnyComponentFlag(uint32_t euid, uint64_t f) {
		return (globals::g_EntityManager->GetEntity(euid).ComponentBitfield & f);
	}

	static bool HasAllComponentFlag(uint32_t euid, uint64_t f) {
		return (globals::g_EntityManager->GetEntity(euid).ComponentBitfield & f) == f;
	}

	static void RegisterEntityInterface() {
		asIScriptEngine* engine = g_ScriptEngine.GetEngine();
		engine->RegisterGlobalFunction("uint CreateEntity(uint tag)", asFUNCTION(&CreateEntity), asCALL_CDECL);
		engine->RegisterGlobalFunction("uint CreateEntity()", asFUNCTION(&CreateEntity), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool EntityHasAnyCompFlag(uint euid, uint64 flag)", asFUNCTION(&HasAnyComponentFlag), asCALL_CDECL);
		engine->RegisterGlobalFunction("bool EntityHasAllCompFlag(uint euid, uint64 flag)", asFUNCTION(&HasAllComponentFlag), asCALL_CDECL);
	}

}