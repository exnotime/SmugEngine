#pragma once
#include <EASTL/string.h>
#include <map>
namespace smug {
class StringPool {
  public:
	void AddToPool(uint32_t hash, eastl::string string);
	void Serialize(const eastl::string& filename);
	void DeSerialize(const eastl::string& filename);
	eastl::string GetString(uint32_t hash) const;
	void Print();
  private:
	std::map<uint32_t, eastl::string> m_Strings;
};
//initialized at start of compiler main
extern StringPool* g_StringPool;
}