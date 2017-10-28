#pragma once
#include <string>
#include "Resources.h"
#include "LoaderInterface.h"

class ShaderLoader : public LoaderInterface {
  public:
	ShaderLoader();
	~ShaderLoader();
	char* LoadShaders(const std::string& filename, ShaderInfo& info);

	virtual LoadResult LoadAsset(const char* filename) override;
	virtual void UnloadAsset(void* asset) override;
	virtual void SerializeAsset(FileBuffer* buffer, LoadResult* asset) override;
	virtual DeSerializedResult DeSerializeAsset(void* assetBuffer) override;
	virtual bool IsExtensionSupported(const char* extension) override;
};