#pragma once
#include "AssetExport.h"
#include "Resources.h"
#include "TextureLoader.h"
#include "ModelLoader.h"
enum ResourceTypes : uint64_t {
	RT_TEXTURE = 1,
	RT_MODEL,
	RT_ANIMATION,
	RT_SKELETON
};

/*
Allocations that need gpu upload need to go through an external allocator since
this lib is API agnostic.
*/
typedef ASSET_DLL ResourceHandle(*AllocateTexture)(const TextureInfo& info, void* userData);
typedef ASSET_DLL ResourceHandle(*AllocateModel)(const ModelInfo& info, void* userData);

struct ASSET_DLL ResourceAllocator {
	AllocateTexture AllocTexture;
	void* TextureData = nullptr;
	AllocateModel AllocModel;
	void* ModelData = nullptr;

};
#define g_AssetLoader AssetLoader::GetInstance()

class ASSET_DLL AssetLoader {
public:
	~AssetLoader();
	void SetResourceAllocator(ResourceAllocator allocator);
	ResourceHandle LoadAsset(const char* filename);

	static AssetLoader* GetInstance() {
		if (!m_Instance) {
			m_Instance = new AssetLoader();
		}
		return m_Instance;
	}
	
private:
	AssetLoader();
	static AssetLoader* m_Instance;

	ResourceAllocator m_Allocator;
	TextureLoader m_TexLoader;
	ModelLoader m_ModelLoader;
};

