#pragma once
#include "AssetExport.h"
#include "Resources.h"
#include <unordered_map>
#include <Utility/StringPool.h>
#include <Utility/Filebuffer.h>
#include "LoaderInterface.h"
namespace smug {
	typedef ASSET_DLL void(*AllocateAsset)(const void* data, void* userData, const std::string& filename, const RESOURCE_TYPE type);

	struct ASSET_DLL ResourceAllocator {
		AllocateAsset AllocResource;
		void* UserData;
	};

#define g_AssetLoader AssetLoader::GetInstance()

	struct ResourceLoader {
		FileBuffer* buffer;
		LoaderInterface* loader;
		std::string LedgerFile;
		std::string BankFile;
	};

	class ASSET_DLL AssetLoader {
	public:
		~AssetLoader();

		void Init(const char* dataFolder, bool isCompiler);
		void Close();
		void SetResourceAllocator(ResourceAllocator allocator);
		ResourceHandle LoadAsset(const char* filename);
		ResourceHandle LoadAsset(uint32_t hash);
		void UnloadAsset(ResourceHandle h);
		void LoadStringPool(const char* filename);
		void SaveStringPool(const char* filename);
		std::string GetFilenameFromCache(ResourceHandle handle);

		static AssetLoader& GetInstance();
	private:
		AssetLoader();
		ResourceAllocator m_Allocator;
		std::unordered_map<uint32_t, ResourceHandle> m_ResourceCache;
		StringPool m_StringPool;
		bool m_IsCompiler = false;
		std::vector<ResourceLoader> m_Loaders;
		//compiler extra
		std::unordered_map< ResourceHandle, std::string> m_FilenameCache;
	};
}

