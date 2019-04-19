#pragma once
#include "AssetExport.h"
#include "Resources.h"
#include <EASTL/unordered_map.h>
#include <EASTL/vector.h>
#include <Utility/StringPool.h>
#include <Utility/Filebuffer.h>
#include "LoaderInterface.h"
namespace smug {
typedef ASSET_DLL void(*AllocateAsset)(const void* data, void* userData, const eastl::string& filename, const RESOURCE_TYPE type);
typedef ASSET_DLL void(*ReAllocateAsset)(const void* data, void* userData, ResourceHandle handle);
typedef ASSET_DLL void(*DeAllocateAsset)(ResourceHandle handle, void* userData);

struct ASSET_DLL ResourceAllocator {
	AllocateAsset AllocResource;
	ReAllocateAsset ReAllocResource;
	DeAllocateAsset DeAllocResource;
	void* UserData;
};

#define g_AssetLoader AssetLoader::GetInstance()

struct ResourceLoader {
	FileBuffer* buffer;
	LoaderInterface* loader;
	eastl::string LedgerFile;
	eastl::string BankFile;
};

class ASSET_DLL AssetLoader {
  public:
	~AssetLoader();

	void Init(const char* dataFolder, bool isCompiler);
	void Close();
	void SetResourceAllocator(ResourceAllocator allocator);
	ResourceHandle LoadAsset(const char* filename);
	ResourceHandle LoadAsset(uint32_t hash);

	ResourceHandle LoadGeneratedModel(ModelInfo& modelInfo, const char* name);
	void UpdateModel(ResourceHandle handle, ModelInfo& modelInfo);

	void UnloadAsset(ResourceHandle h);
	void UnloadAllAssets();
	void LoadStringPool(const char* filename);
	void SaveStringPool(const char* filename);
	eastl::string GetFilenameFromCache(ResourceHandle handle);
	StringPool& GetStringPool() { return m_StringPool; };
	static AssetLoader& GetInstance();
  private:
	AssetLoader();
	ResourceAllocator m_Allocator;
	eastl::unordered_map<uint32_t, ResourceHandle> m_ResourceCache;
	eastl::unordered_map<ResourceHandle, void*> m_ResourceData;
	StringPool m_StringPool;
	bool m_IsCompiler = false;
	eastl::vector<ResourceLoader> m_Loaders;
	//compiler extra
	eastl::unordered_map< ResourceHandle, eastl::string> m_FilenameCache;
};
}

