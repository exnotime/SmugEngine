#ifndef ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_H
#define ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_H

#include <stdint.h>
#include "angelscript-integration.h"

namespace AngelScriptIntegration {

class RefCountedObject
{
public:
  RefCountedObject(const RefCountedObject&) = delete;
  RefCountedObject(RefCountedObject&&) = delete;
  RefCountedObject& operator=(const RefCountedObject&) = delete;
  RefCountedObject& operator=(RefCountedObject&&) = delete;

  void addReference();
  void releaseReference();

  AngelScript::asILockableSharedBool* get_weakrefFlag();

protected:
  RefCountedObject();
  virtual ~RefCountedObject();

  template<typename T>
  static void registerAsBaseOfClass(AngelScript::asIScriptEngine* engine, const char* className);

private:
  int referenceCount = 1;
  AngelScript::asILockableSharedBool* weakRefFlag = nullptr;
};

template<typename T>
class WrapWithRefCounter : public RefCountedObject
{
public:
  typedef WrapWithRefCounter<T> wrapper_type;
  T value;

  static void registerClass(AngelScript::asIScriptEngine* engine, const char* className, bool registerDefaultConstructor=true, bool registerType=true);

private:
  static wrapper_type* create();
};


} // namespace AngelScriptIntegration

#include "./ref-counted-object.inl"

#endif // ANGELSCRIPTINTEGRATION_REFCOUNTEDOBJECT_H
