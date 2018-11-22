#include "ScriptEngine.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"

#include "scripthelper/scripthelper.h"
#include "scriptmatrix.h"
#include <angelscript-integration/angelscript-integration.h>
#include <assert.h>
#include <glm/glm.hpp>


using namespace smug;
void MessageCallback(const AngelScript::asSMessageInfo *msg, void *param) {
	const char *type = "ERR ";
	if (msg->type == AngelScript::asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == AngelScript::asMSGTYPE_INFORMATION)
		type = "INFO";
	printf("%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void Print(const std::string& msg) {
	printf("%s\n", msg.c_str());
}

void RegisterMatrixes(AngelScript::asIScriptEngine* engine) {
	using namespace AngelScript;
	engine->RegisterObjectType("mat4x4", sizeof(Mat4x4), asOBJ_VALUE | asGetTypeTraits<Mat4x4>());
	engine->RegisterObjectBehaviour("mat4x4", asBEHAVE_CONSTRUCT, "void mat4x4()", asFUNCTION(ConstructMat4x4), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("mat4x4", asBEHAVE_DESTRUCT, "void mat4x4()", asFUNCTION(DestructMat4x4), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("mat4x4", "vec4 opMul(vec4) const", asMETHODPR(Mat4x4, operator*, (const glm::vec4&) const, glm::vec4), asCALL_THISCALL);
	engine->RegisterObjectMethod("mat4x4", "vec4 opIndex(uint) const", asMETHODPR(Mat4x4, operator[], (const unsigned int), glm::vec4&), asCALL_THISCALL);

	engine->RegisterObjectType("mat3x4", sizeof(Mat3x4), asOBJ_VALUE | asGetTypeTraits<Mat3x4>());
	engine->RegisterObjectBehaviour("mat3x4", asBEHAVE_CONSTRUCT, "void mat3x4()", asFUNCTION(ConstructMat3x4), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("mat3x4", asBEHAVE_DESTRUCT, "void mat3x4()", asFUNCTION(DestructMat3x4), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("mat3x4", "vec4 opMul(vec4) const", asMETHODPR(Mat3x4, operator*, (const glm::vec4&) const, glm::vec4), asCALL_THISCALL);
	engine->RegisterObjectMethod("mat3x4", "vec4 opIndex(uint) const", asMETHODPR(Mat3x4, operator[], (const unsigned int), glm::vec4&), asCALL_THISCALL);

	engine->RegisterObjectType("mat3x3", sizeof(Mat3x3), asOBJ_VALUE | asGetTypeTraits<Mat3x3>());
	engine->RegisterObjectBehaviour("mat3x3", asBEHAVE_CONSTRUCT, "void mat3x3()", asFUNCTION(ConstructMat3x3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectBehaviour("mat3x3", asBEHAVE_DESTRUCT, "void mat3x3()", asFUNCTION(DestructMat3x3), asCALL_CDECL_OBJLAST);
	engine->RegisterObjectMethod("mat3x3", "vec3 opMul(vec3) const", asMETHODPR(Mat3x3, operator*, (const glm::vec3&) const, glm::vec3), asCALL_THISCALL);
	engine->RegisterObjectMethod("mat3x3", "vec3 opIndex(uint) const", asMETHODPR(Mat3x3, operator[], (const unsigned int), glm::vec3&), asCALL_THISCALL);
}

ScriptEngine::ScriptEngine() {
}

ScriptEngine::~ScriptEngine() {
	m_Engine->GarbageCollect();
	m_Context->Release();
	m_Engine->ShutDownAndRelease();
}

ScriptEngine& ScriptEngine::GetInstance() {
	static ScriptEngine m_Instance;
	return m_Instance;
}

void ScriptEngine::Init() {
	m_Engine = AngelScript::asCreateScriptEngine();
	m_Engine->SetMessageCallback(asFUNCTION(MessageCallback), nullptr, AngelScript::asCALL_CDECL);
	RegisterStdString(m_Engine);
	RegisterScriptArray(m_Engine, true);
	m_Engine->RegisterGlobalFunction("void print(const string &in)", AngelScript::asFUNCTION(Print), AngelScript::asCALL_CDECL);
	AngelScriptIntegration::init_glm(m_Engine, AngelScriptIntegration::GlmFlags::NO_SWIZZLE);
	//glm integration has no matrix type
	RegisterMatrixes(m_Engine);
	m_Context = m_Engine->CreateContext();
}

void ScriptEngine::CompileScript(const std::string& scriptname) {
	AngelScript::CScriptBuilder scriptBuilder;
	int r = 0;
	r = scriptBuilder.StartNewModule(m_Engine, scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.AddSectionFromFile(scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.BuildModule();
	assert(r >= 0);
	m_Scripts.push_back(scriptname);
	printf("Compiled script :%s\n", scriptname.c_str());
}

AngelScript::asIScriptModule* ScriptEngine::CompileScriptToModule(const std::string& scriptname) {
	AngelScript::CScriptBuilder scriptBuilder;
	int r = 0;
	r = scriptBuilder.StartNewModule(m_Engine, scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.AddSectionFromFile( scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.BuildModule();
	assert(r >= 0);
	m_Scripts.push_back(scriptname);
	printf("Compiled script :%s\n", scriptname.c_str());
	return m_Engine->GetModule(scriptname.c_str());
}

AngelScript::asIScriptModule* ScriptEngine::CompileStringToModule(const std::string& name, const std::string& script) {
	AngelScript::CScriptBuilder scriptBuilder;
	int r = 0;
	r = scriptBuilder.StartNewModule(m_Engine, name.c_str());
	assert(r >= 0);
	r = scriptBuilder.AddSectionFromMemory(name.c_str(), script.c_str());
	assert(r >= 0);
	r = scriptBuilder.BuildModule();
	assert(r >= 0);
	m_Scripts.push_back(name);
	printf("Compiled script :%s\n", name.c_str());
	return m_Engine->GetModule(name.c_str());
}

void ScriptEngine::RunScript(const std::string& scriptname, const std::string& entry) {
	AngelScript::asIScriptModule* module = m_Engine->GetModule(scriptname.c_str());
	AngelScript::asIScriptFunction* func = module->GetFunctionByDecl(entry.c_str());
	m_Context->Prepare(func);
	m_Context->Execute();
}

void ScriptEngine::RecompileScript(const std::string& scriptname) {
	m_Engine->DiscardModule(scriptname.c_str());
	AngelScript::CScriptBuilder scriptBuilder;
	int r = 0;
	r = scriptBuilder.StartNewModule(m_Engine, scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.AddSectionFromFile(scriptname.c_str());
	assert(r >= 0);
	r = scriptBuilder.BuildModule();
	assert(r >= 0);
}

void ScriptEngine::RecompileAllScripts() {
	for (auto& script : m_Scripts) {
		RecompileScript(script);
	}
}

void ScriptEngine::ExecuteString(const std::string& code) {
	AngelScript::ExecuteString(m_Engine, code.c_str(), nullptr, m_Context);
}

void ScriptEngine::ExecuteModule(AngelScript::asIScriptModule* module, const std::string& entry) {
	AngelScript::asIScriptFunction* func = module->GetFunctionByDecl(entry.c_str());
	m_Context->Prepare(func);
	m_Context->Execute();
}