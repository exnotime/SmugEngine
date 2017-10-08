#pragma once
#include <string>
#include <vector>
#include <unordered_map>

struct LedgerEntry {
	uint64_t TimeStamp;
	size_t Offset;
	size_t Size;
	uint32_t Hash;
	uint32_t padding;
};

class FileBuffer {
public:
	FileBuffer();
	~FileBuffer();
	void Open(const char* filename, const char* ledgerName, size_t initialSize = 1024 * 1024);
	void Close();
	size_t Write(size_t size, void* data, const std::string& filename);
	void Flush();
	size_t GetWritePtr() { return m_FilePtr + m_Ptr; }
private:
	uint8_t* m_Buffer;
	size_t m_Size;
	size_t m_Ptr;
	bool m_Open;
	FILE* m_File;
	FILE* m_LedgerFile;
	std::string m_Filename;
	std::string m_LedgerFilename;
	size_t m_FilePtr;
	std::unordered_map<uint32_t, LedgerEntry> m_Files;
};