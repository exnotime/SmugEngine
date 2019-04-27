#pragma once
#include <angelscript.h>
#include <EASTL/string.h>
#include <EASTL/vector.h>
#include "scriptarray/scriptarray.h"
#include "ScriptExport.h"
#define g_ScriptEngine ScriptEngine::GetInstance()
namespace smug {
class ScriptEngine {
  public:
	~ScriptEngine();
	static ScriptEngine& GetInstance();
	void Init();
	void CompileScript(const eastl::string& scriptname);
	AngelScript::asIScriptModule* CompileScriptToModule(const eastl::string& scriptname);
	AngelScript::asIScriptModule* CompileStringToModule(const eastl::string& name, const eastl::string& script);
	void RunScript(const eastl::string& scriptname, const eastl::string& entry);
	AngelScript::asIScriptEngine* GetEngine() {
		return m_Engine;
	}
	AngelScript::asIScriptContext* GetContext() {
		return m_Context;
	}
	void RecompileScript(const eastl::string& scriptname);
	void RecompileAllScripts();
	void ExecuteString(const eastl::string& code);
	void ExecuteModule(AngelScript::asIScriptModule* module, const eastl::string& entry);
  private:
	ScriptEngine();
	eastl::vector<eastl::string> m_Scripts;
	AngelScript::asIScriptEngine* m_Engine;
	AngelScript::asIScriptContext* m_Context;
};
}