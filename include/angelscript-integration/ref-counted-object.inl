#ifndef ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_INL
#define ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_INL

#include "./ref-counted-object.h"

namespace AngelScriptIntegration {


template<typename T>
void RefCountedObject::registerAsBaseOfClass(AngelScript::asIScriptEngine* engine, const char* className)
{
  static_assert(std::is_base_of<RefCountedObject, T>::value, "T must inherit from AngelScriptIntegration::RefCountedObject");

  int r = 0;

  r = engine->RegisterObjectBehaviour(className, AngelScript::asBEHAVE_ADDREF, "void f()", AngelScript::asMETHOD(T,addReference), AngelScript::asCALL_THISCALL); AngelScriptCheck(r);
  r = engine->RegisterObjectBehaviour(className, AngelScript::asBEHAVE_RELEASE, "void f()", AngelScript::asMETHOD(T,releaseReference), AngelScript::asCALL_THISCALL); AngelScriptCheck(r);
  r = engine->RegisterObjectBehaviour(className, AngelScript::asBEHAVE_GET_WEAKREF_FLAG, "int& f()", AngelScript::asMETHOD(T,get_weakrefFlag), AngelScript::asCALL_THISCALL); AngelScriptCheck(r);
}


template<typename T>
void WrapWithRefCounter<T>::registerClass(AngelScript::asIScriptEngine* engine, const char* className, bool registerDefaultConstructor, bool registerType)
{
  int r = 0;

  if(registerType)
  {
    r = engine->RegisterObjectType(className, 0, AngelScript::asOBJ_REF);
    AngelScriptCheck(r);
  }
  if(registerDefaultConstructor)
  {
    r = engine->RegisterObjectBehaviour(className, AngelScript::asBEHAVE_FACTORY, (std::string(className)+"@ f()").c_str(), AngelScript::asFUNCTION(create), AngelScript::asCALL_CDECL);
    AngelScriptCheck(r);
  }

  RefCountedObject::registerAsBaseOfClass<wrapper_type>(engine, className);
}

template<typename T>
WrapWithRefCounter<T>* WrapWithRefCounter<T>::create()
{
  return new WrapWithRefCounter<T>;
}


} // namespace AngelScriptIntegration

#endif // ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_INL
