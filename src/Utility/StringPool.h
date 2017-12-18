#pragma once
#include <string>
#include <map>

class StringPool {
public:
	void AddToPool(uint32_t hash, std::string string);
	void Serialize(const std::string& filename);
	void DeSerialize(const std::string& filename);
	std::string GetString(uint32_t hash) const;
	void Print();
private:
	std::map<uint32_t, std::string> m_Strings;
};

extern StringPool* g_StringPool;