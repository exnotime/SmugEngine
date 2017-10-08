#pragma once
#include "AssetExport.h"
#include "Resources.h"
#include <unordered_map>

enum ResourceTypes : uint64_t {
	RT_TEXTURE = 0x1,
	RT_MODEL = 0x2,
	RT_SHADER = 0x4,
	RT_ANIMATION = 0x8,
	RT_SKELETON = 0x10,
	RT_SCRIPT = 0x20,
	RT_LEVEL = 0x40,
	RT_ALL_TYPES = 0xff
};

typedef ASSET_DLL ResourceHandle(*AllocateTexture)(const TextureInfo& info, void* userData, const std::string& filename);
typedef ASSET_DLL ResourceHandle(*AllocateModel)(const ModelInfo& info, void* userData, const std::string& filename);
typedef ASSET_DLL ResourceHandle(*AllocateShader)(const ShaderInfo& info, void* userData, const std::string& filename);


struct ASSET_DLL ResourceAllocator {
	AllocateTexture AllocTexture;
	void* TextureData = nullptr;
	AllocateModel AllocModel;
	void* ModelData = nullptr;
	AllocateShader AllocShader;
	void* ShaderData = nullptr;
};

#define g_AssetLoader AssetLoader::GetInstance()

class TextureLoader;
class ModelLoader;
class ShaderLoader;

class ASSET_DLL AssetLoader {
  public:
	~AssetLoader();
	void SetResourceAllocator(ResourceAllocator allocator);
	ResourceHandle LoadAsset(const char* filename);
	void* GetAsset(ResourceHandle handle, ResourceTypes type = RT_ALL_TYPES);
	void UnloadAsset(ResourceHandle h);
	void Clear();

	static AssetLoader& GetInstance();
  private:
	AssetLoader();

	ResourceAllocator m_Allocator;
	TextureLoader* m_TexLoader;
	ModelLoader* m_ModelLoader;
	ShaderLoader* m_ShaderLoader;
	std::unordered_map<std::string, ResourceHandle> m_ResourceCache;

};

