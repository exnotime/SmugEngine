#ifndef ANGELSCRIPTINTEGRATION_CALLSCRIPT_INL
#define ANGELSCRIPTINTEGRATION_CALLSCRIPT_INL

#include "call-script.h"
#include "change-working-dir.h"

#include <QFileInfo>

namespace AngelScriptIntegration {
namespace Implementation {

class ScriptContextReleaser
{
  AngelScript::asIScriptContext* const context;
public:

  ScriptContextReleaser(AngelScript::asIScriptContext* context)
    : context(context)
  {
  }

  ~ScriptContextReleaser()
  {
    context->Release();
  }
};

inline void _pass_arguments_to_angelscript(AngelScript::asIScriptContext*, int)
{
}

template<typename T_arg_i, typename... T_args>
void _pass_arguments_to_angelscript(AngelScript::asIScriptContext* context, int i, const T_arg_i& arg_i, const T_args&... other_args)
{
  pass_arg_to_angelscript(context, i, arg_i);

  _pass_arguments_to_angelscript(context, i+1, other_args...);
}

} // namespace Implementation

void CheckExecutionResult(AngelScript::asIScriptContext* context, int r);


template<typename T_return, typename... T_args>
T_return callScriptFunction(AngelScript::asIScriptFunction* function, const T_args&... args)
{
  Q_ASSERT(function != nullptr);
  AngelScript::asIScriptEngine* engine = function->GetEngine();

  AngelScript::asIScriptContext* context = engine->CreateContext();

  // Using the destructor to release the context to allow returning the return value directly
  Implementation::ScriptContextReleaser scriptContextReleaser(context);
  Q_UNUSED(scriptContextReleaser);

  context->Prepare(function);

  Implementation::_pass_arguments_to_angelscript(context, 0, args...);

  int r = context->Execute();

  CheckExecutionResult(context, r);

  return ResultFromAngelScript<T_return>::value(context);
}


template<typename T_return, typename... T_args>
T_return callScriptExt(AngelScript::asIScriptEngine* engine,
                       const std::string& filepath,
                       const char* functionDeclarationToCall,
                       const char* preferredModuleName,
                       const ConfigCallScript& config,
                       const T_args&... args)
{
  QFileInfo file(QString::fromStdString(filepath));

  std::string absoluteFilepath = file.absoluteFilePath().toStdString();

  ChangeWorkingDir cd(file.dir());

  std::string moduleName = getUniqueModuleName(engine, preferredModuleName);
  AngelScript::asIScriptModule* module = loadAndCompileModule(engine, absoluteFilepath.c_str(), moduleName.c_str(), config);

  AngelScript::asIScriptFunction* function = module->GetFunctionByDecl(functionDeclarationToCall);

  Q_ASSERT(function != nullptr);
  Q_UNUSED(cd);

  return callScriptFunction<T_return>(function, args...);
}


template<typename T_return, typename... T_args>
T_return callScript(AngelScript::asIScriptEngine* engine,
                    const std::string& filepath,
                    const char* functionDeclarationToCall,
                    const char* preferredModuleName,
                    const T_args&... args)
{
  return callScriptExt<T_return>(engine, filepath, functionDeclarationToCall, preferredModuleName, ConfigCallScript(), args...);
}



} // namespace AngelScriptIntegration

#endif // ANGELSCRIPTINTEGRATION_CALLSCRIPT_INL

