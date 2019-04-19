#include "StringPool.h"
#include <stdio.h>
#include <EASTL/vector.h>

using namespace smug;
StringPool* g_StringPool = nullptr;

void StringPool::AddToPool(uint32_t hash, eastl::string string) {
	if (m_Strings.find(hash) == m_Strings.end()) {
		m_Strings[hash] = string;
	}
}

void StringPool::Serialize(const eastl::string& filename) {
	FILE* fout = fopen(filename.c_str(), "wb");
	if (!fout)
		return;
	struct StringHeader {
		uint32_t Hash;
		uint32_t Length;
		uint32_t Offset;
		uint32_t Padding;
	};

	eastl::vector<StringHeader> headers;
	eastl::vector<eastl::string> strings; //keep string in this list to ensure that they are in order
	uint64_t offset = sizeof(uint32_t) + sizeof(StringHeader) * m_Strings.size();

	for (auto& s : m_Strings) {
		StringHeader h;
		h.Hash = s.first;
		h.Length = (uint32_t)s.second.length() + 1;
		h.Offset = (uint32_t)offset;
		offset += h.Length;
		headers.push_back(h);
		strings.push_back(s.second);
	}
	uint32_t count = (uint32_t)headers.size();
	fwrite(&count, sizeof(uint32_t), 1, fout);
	fwrite(headers.data(), sizeof(StringHeader), headers.size(), fout);
	for (auto& s : strings) {
		fwrite(s.c_str(), sizeof(char), s.length() + 1, fout);
	}
	fclose(fout);
}

void StringPool::DeSerialize(const eastl::string& filename) {
	FILE* fin = fopen(filename.c_str(), "rb");
	if (!fin)
		return;

	struct StringHeader {
		uint32_t Hash;
		uint32_t Length;
		uint32_t Offset;
		uint32_t Padding;
	};

	uint32_t headerCount;
	fread(&headerCount, sizeof(uint32_t), 1, fin);
	eastl::vector<StringHeader> headers;
	for (uint32_t i = 0; i < headerCount; ++i) {
		StringHeader sh;
		fread(&sh, sizeof(StringHeader), 1, fin);
		headers.push_back(sh);
	}

	fseek(fin, 0, SEEK_SET);
	for (auto& h : headers) {
		fseek(fin, h.Offset, SEEK_SET);
		char* buffer = (char*)malloc(h.Length);
		fread(buffer, sizeof(char), h.Length, fin);
		m_Strings[h.Hash] = eastl::string(buffer);
		free(buffer);
	}
	fclose(fin);

}

eastl::string StringPool::GetString(uint32_t hash) const {
	auto& s = m_Strings.find(hash);
	if (s != m_Strings.end())
		return s->second;
	else
		return "";
}

void StringPool::Print() {
	for (auto& s : m_Strings)
		printf("Hash: %xz String: %s\n", s.first, s.second.c_str());
}