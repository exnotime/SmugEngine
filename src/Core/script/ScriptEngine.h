#pragma once
#include <angelscript.h>
#include <string>
#include <vector>
#include "scriptarray/scriptarray.h"
#define g_ScriptEngine ScriptEngine::GetInstance()
namespace smug {
class ScriptEngine {
  public:
	~ScriptEngine();
	static ScriptEngine& GetInstance();
	void Init();
	void CompileScript(const std::string& scriptname);
	AngelScript::asIScriptModule* CompileScriptToModule(const std::string& scriptname);
	AngelScript::asIScriptModule* CompileStringToModule(const std::string& name, const std::string& script);
	void RunScript(const std::string& scriptname, const std::string& entry);
	AngelScript::asIScriptEngine* GetEngine() {
		return m_Engine;
	}
	void RecompileScript(const std::string& scriptname);
	void RecompileAllScripts();
	void ExecuteString(const std::string& code);
	void ExecuteModule(AngelScript::asIScriptModule* module, const std::string& entry);
  private:
	ScriptEngine();
	std::vector<std::string> m_Scripts;
	AngelScript::asIScriptEngine* m_Engine;
	AngelScript::asIScriptContext* m_Context;
};
}