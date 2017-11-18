#include "Filebuffer.h"
#include <stdio.h>
#include <functional> //std::hash
#include <Utility/MurmurHash3.h>

FileBuffer::FileBuffer() {
	m_Size = 0;
	m_Ptr = 0;
	m_FilePtr = 0;
	m_Open = false;
}
FileBuffer::~FileBuffer() {

}
void FileBuffer::OpenForWriting(const char* filename, const char* ledgerName, size_t initialSize) {
	m_File = fopen(filename, "wb");
	if (!m_File) {
		printf("Unable to open file %s\n", filename);
		return;
	}
	m_LedgerFile = fopen(ledgerName, "wb");
	if (!m_LedgerFile) {
		printf("Unable to open file %s\n", ledgerName);
		return;
	}
	m_Buffer = (uint8_t*)malloc(initialSize);
	if (!m_Buffer) {
		printf("Unable to allocate %zu bytes \n", initialSize);
		return;
	}

	m_Size = initialSize;
	m_Open = true;
	m_LedgerFilename = ledgerName;
	m_Filename = filename;
}

void FileBuffer::Close() {
	Flush();
	if (m_FilePtr > 0) {
		fclose(m_File);
		fclose(m_LedgerFile);
		free(m_Buffer);
	}

	m_Open = false;
}

size_t FileBuffer::Write(size_t size, void* data, uint32_t hash) {
	if (!m_Open) {
		return 0;
	}
	//if we dont have room for more expand until we eithe run out of memory of flush to disk
	while (m_Ptr + size >= m_Size) {
		//Try to increase buffer size
		uint8_t* newBuffer = (uint8_t*)realloc(m_Buffer, m_Size * 2);
		if (!newBuffer) {
			//unable to increase buffersize flush current content to disk and continue
			Flush();
		}
		m_Size *= 2;
		m_Buffer = newBuffer;
	}
	auto& f = m_Files.find(hash);
	if (f == m_Files.end()) {
		LedgerEntry newFile;
		newFile.Size = size;
		newFile.Offset = m_FilePtr + m_Ptr;
		newFile.Hash = hash;
		m_Files[hash] = newFile;
	} else {
		m_Files[hash].Size += size;
	}

	memcpy(m_Buffer + m_Ptr, data, size);

	m_Ptr += size;
	return m_FilePtr + m_Ptr;
}

void FileBuffer::Flush() {
	if (m_Open) {
		//write out files
		fwrite(m_Buffer, sizeof(unsigned char), m_Ptr, m_File);
		for (auto& f : m_Files) {
			fwrite(&f.second, sizeof(LedgerEntry), 1, m_LedgerFile);
		}
		m_Files.clear();
		//this will update the files on disk
		fflush(m_File);
		fflush(m_LedgerFile);

		m_FilePtr += m_Ptr; //add on the amount we have written now so we can track it in the ledger
		m_Ptr = 0; //turn back writehead
		m_Files.clear();
	}
}

void FileBuffer::OpenForReading(const char* filename, const char* ledgerFile) {
	//read ledger
	FILE* f = fopen(ledgerFile, "rb");
	if (f) {
		LedgerEntry entry;
		while (fread(&entry, sizeof(LedgerEntry), 1, f) == 1) {
			if (feof(f))
				break;
			m_Files.insert({ entry.Hash, entry });
		}
		fclose(f);
	}
	m_LedgerFilename = ledgerFile;
	m_Filename = filename;
	m_Open = true;
}

void* FileBuffer::LoadFile(uint32_t hash) {
	auto& e = m_Files.find(hash);
	if (e == m_Files.end())
		return nullptr;
	
	m_File = fopen(m_Filename.c_str(), "rb");
	if (!m_File)
		return nullptr;

	printf("loading file %u, at offset %llu of size %llu", hash, e->second.Offset, e->second.Size);

	fseek(m_File, e->second.Offset, SEEK_SET);
	void* buffer = malloc(e->second.Size);
	fread(buffer, sizeof(uint8_t), e->second.Size, m_File);
	fclose(m_File);
	return buffer;
}