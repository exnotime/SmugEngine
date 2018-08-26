#pragma once
#include <string>
#include <Utility/Filebuffer.h>
namespace smug {
struct LoadResult {
	std::string Error = "Unknown";
	void* Data = nullptr;
	uint32_t Hash;
	RESOURCE_TYPE Type;
};

struct DeSerializedResult {
	void* Data;
	RESOURCE_TYPE Type;
};

class LoaderInterface {
  public:
	virtual LoadResult LoadAsset(const char* filename) = 0;
	virtual void UnloadAsset(void* asset) = 0;
	virtual void SerializeAsset(FileBuffer* buffer, LoadResult* asset) = 0;
	virtual DeSerializedResult DeSerializeAsset(void* assetBuffer) = 0;
	virtual bool IsExtensionSupported(const char* extension) = 0;
	virtual bool IsTypeSupported(int type) = 0;
};
}