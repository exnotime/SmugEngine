#pragma once
#include <string>
#include "Resources.h"
#include "AssetExport.h"
#include "LoaderInterface.h"

class ModelLoader : public LoaderInterface {
  public:
	ModelLoader();
	~ModelLoader();
	char* LoadModel(const std::string& filename, ModelInfo& info);

	virtual LoadResult LoadAsset(const char* filename) override;
	virtual void UnloadAsset(void* asset) override;
	virtual void SerializeAsset(FileBuffer* buffer, LoadResult* asset) override;
	virtual DeSerializedResult DeSerializeAsset(void* assetBuffer) override;
	virtual bool IsExtensionSupported(const char* extension) override;
};