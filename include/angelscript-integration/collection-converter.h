#ifndef ANGELSCRIPTINTEGRATION_COLLECTIONCONVERTER_H
#define ANGELSCRIPTINTEGRATION_COLLECTIONCONVERTER_H

#include <sdk/add_on/scriptarray/scriptarray.h>
#include <sdk/add_on/scriptdictionary/scriptdictionary.h>

#include <QSet>
#include <QVector>
#include <QDebug>

namespace AngelScriptIntegration {

template<typename T>
QSet<T> scriptArrayToSet(AngelScript::CScriptArray* array)
{
  QSet<T> set;

  const size_t n = array->GetSize();

  for(size_t i=0; i<n; ++i)
    set.insert(*reinterpret_cast<T*>(array->At(i)));

  return set;
}

inline QSet<QString> scriptArrayToStringSet(AngelScript::CScriptArray* array)
{
  QSet<QString> set;

  const size_t n = array->GetSize();

  for(size_t i=0; i<n; ++i)
    set.insert(QString::fromStdString(*reinterpret_cast<std::string*>(array->At(i))));

  return set;
}

template<typename T>
AngelScript::CScriptArray* scriptArrayFromVector(const QVector<T>& vector, AngelScript::asIScriptEngine* engine, const std::string& innerType)
{
  AngelScript::asIObjectType* arrayType = engine->GetObjectTypeByDecl(("array<"+innerType+">").c_str());

  Q_ASSERT(arrayType != nullptr);

  AngelScript::CScriptArray* array = AngelScript::CScriptArray::Create(arrayType, vector.length());

  for(int i=0; i<vector.length(); ++i)
  {
    T copy = vector[i];
    array->SetValue(i, &copy);
  }

  return array;
}

inline AngelScript::CScriptArray* scriptArrayFromStringSet(QSet<QString> set, AngelScript::asIScriptEngine* engine)
{
  QVector<std::string> convertedSet;
  convertedSet.reserve(set.size());

  for(const QString& s : set)
    convertedSet.append(s.toStdString());

  return scriptArrayFromVector<std::string>(convertedSet, engine, "string");
}

template<typename T>
QHash<QString, T> scriptDictionaryToHash(AngelScript::CScriptDictionary* dict, const QSet<int>& typeIds, AngelScript::asIScriptEngine* engine=nullptr)
{
  QHash<QString, T> hash;

  for(AngelScript::CScriptDictionary::CIterator i=dict->begin(); i!=dict->end(); ++i)
  {
    QString key = QString::fromStdString(i.GetKey());

    int i_type = i.GetTypeId();
    if(typeIds.contains(i_type))
    {
      T v;
      if(!i.GetValue(&v, i_type))
      {
        Q_UNREACHABLE();
      }
      hash[key] = v;
    }else
    {
      if(engine)
        qWarning() << "scriptDictionaryToHash: Unexpected type: " << engine->GetTypeDeclaration(i_type, true);
      Q_UNREACHABLE();
    }
  }

  return hash;
}

template<typename T>
QHash<QString, T> scriptDictionaryToFloatHash(AngelScript::CScriptDictionary* dict)
{
  QHash<QString, T> hash;

  for(AngelScript::CScriptDictionary::CIterator i=dict->begin(); i!=dict->end(); ++i)
  {
    QString key = QString::fromStdString(i.GetKey());

    double doubleValue;
    i.GetValue(doubleValue);
    hash[key] = doubleValue;
  }

  return hash;
}

template<typename T>
AngelScript::CScriptDictionary* scriptDictionaryFromHash(const QHash<QString, T>& hash, int typeId, AngelScript::asIScriptEngine* engine)
{
  AngelScript::CScriptDictionary* dict = AngelScript::CScriptDictionary::Create(engine);

  for(auto i = hash.begin(); i!=hash.end(); ++i)
  {
    std::string key = i.key().toStdString();
    T value = i.value();
    dict->Set(key, &value, typeId);
  }

  return dict;
}

} // namespace AngelScriptIntegration

#endif // ANGELSCRIPTINTEGRATION_COLLECTIONCONVERTER_H
