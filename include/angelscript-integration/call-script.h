#ifndef ANGELSCRIPTINTEGRATION_CALLSCRIPT_H
#define ANGELSCRIPTINTEGRATION_CALLSCRIPT_H

#include "angelscript-integration.h"

#include <QDir>
#include <QVector>

namespace AngelScriptIntegration {

template<class T>
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, T* value)
{
  static_assert(std::is_class<T>::value, "only classes accepted here");

  context->SetArgObject(i, value);
}

void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, const void* value) = delete;
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, bool value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, qint8 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, quint8 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, qint16 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, quint16 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, qint32 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, quint32 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, qint64 value);
void pass_arg_to_angelscript(AngelScript::asIScriptContext*context, int i, quint64 value);

template<typename T_result>
class ResultFromAngelScript;

template<>
class ResultFromAngelScript<void>
{
public:
  static void value(AngelScript::asIScriptContext*)
  {
  }
};

template<>
class ResultFromAngelScript<bool>
{
public:
  static bool value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnByte() != 0;
  }
};

template<>
class ResultFromAngelScript<qint8>
{
public:
  static qint8 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnByte();
  }
};

template<>
class ResultFromAngelScript<quint8>
{
public:
  static quint8 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnByte();
  }
};

template<>
class ResultFromAngelScript<qint16>
{
public:
  static qint16 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnWord();
  }
};

template<>
class ResultFromAngelScript<quint16>
{
public:
  static quint16 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnWord();
  }
};

template<>
class ResultFromAngelScript<qint32>
{
public:
  static qint32 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnDWord();
  }
};

template<>
class ResultFromAngelScript<quint32>
{
public:
  static quint32 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnDWord();
  }
};

template<>
class ResultFromAngelScript<qint64>
{
public:
  static qint64 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnQWord();
  }
};

template<>
class ResultFromAngelScript<quint64>
{
public:
  static quint64 value(AngelScript::asIScriptContext* context)
  {
    return context->GetReturnQWord();
  }
};

struct ConfigCallScript
{
  asDWORD accessMask = 0;
  QVector<QDir> includeDirectories;
};

AngelScript::asIScriptModule* loadAndCompileModule(AngelScript::asIScriptEngine* engine, const char* filepath, const char* moduleName, const ConfigCallScript& config);
std::string getUniqueModuleName(AngelScript::asIScriptEngine* engine, const char* preferredModuleName);

template<typename T_return, typename... T_args>
T_return callScript(AngelScript::asIScriptEngine* engine, const std::string& filepath, const char* functionDeclarationToCall, const char* preferredModuleName, const T_args&... args);

template<typename T_return, typename... T_args>
T_return callScriptExt(AngelScript::asIScriptEngine* engine, const std::string& filepath, const char* functionDeclarationToCall, const char* preferredModuleName, const ConfigCallScript& config, const T_args&... args);

template<typename T_return, typename... T_args>
T_return callScriptFunction(AngelScript::asIScriptFunction* function, const T_args&... args);

} // namespace AngelScriptIntegration

#include "call-script.inl"

#endif // ANGELSCRIPTINTEGRATION_CALLSCRIPT_H
