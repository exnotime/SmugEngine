#pragma once
#include <EASTL/string.h>
#include "Resources.h"
#include "LoaderInterface.h"

namespace smug {
class ShaderLoader : public LoaderInterface {
  public:
	ShaderLoader();
	~ShaderLoader();

	virtual LoadResult LoadAsset(const char* filename) override;
	virtual void UnloadAsset(void* asset) override;
	virtual void SerializeAsset(FileBuffer* buffer, LoadResult* asset) override;
	virtual DeSerializedResult DeSerializeAsset(void* assetBuffer) override;
	virtual bool IsExtensionSupported(const char* extension) override;
	virtual bool IsTypeSupported(int type) override;
};
}