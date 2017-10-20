#include "AssetLoader.h"
#include <string>
#include <stdio.h>
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include <Utility/Hash.h>
#define SAFE_DELETE(x) if(x) delete x

AssetLoader::AssetLoader() {
	m_TexLoader = new TextureLoader();
	m_ModelLoader = new ModelLoader();
	m_ShaderLoader = new ShaderLoader();
	m_ResourceCache.reserve(1);
}

AssetLoader::~AssetLoader() {
	SAFE_DELETE(m_TexLoader);
	SAFE_DELETE(m_ModelLoader);
	SAFE_DELETE(m_ShaderLoader);
}

AssetLoader& AssetLoader::GetInstance() {
	static AssetLoader instance;
	return instance;
}

void AssetLoader::SetResourceAllocator(ResourceAllocator allocator) {
	m_Allocator = allocator;
}

bool IsTexture(const std::string& ext) {
	const std::string textureTypes[] = { "dds", "ktx" };
	for (auto& e : textureTypes) {
		if (e == ext)
			return true;
	}
	return false;
}

bool IsModel(const std::string& ext) {
	const std::string modelTypes[] = { "dae", "obj" };
	for (auto& e : modelTypes) {
		if (e == ext)
			return true;
	}
	return false;
}

bool IsShader(const std::string& ext) {
	const std::string shaderTypes[] = { "shader" };
	for (auto& e : shaderTypes) {
		if (e == ext)
			return true;
	}
	return false;
}

ResourceHandle AssetLoader::LoadAsset(const char* filename) {
	ResourceHash hash = HashString(filename);

	if (m_ResourceCache.find(hash) != m_ResourceCache.end()) {
		return m_ResourceCache.find(hash)->second;
	}

	m_StringPool.AddToPool(hash, filename);

	std::string file(filename);
	std::string extention = file.substr(file.find_last_of('.') + 1);
	char* error = nullptr;
	if (IsTexture(extention)) {
		TextureInfo texInfo;
		error = m_TexLoader->LoadTexture(file, texInfo);
		if (!error) {
			ResourceHandle handle = m_Allocator.AllocTexture(texInfo, m_Allocator.TextureData, filename);
			handle |= (RT_TEXTURE << RESOURCE_INDEX_SHIFT);
			m_ResourceCache[hash] = handle;
			//clean up
			SAFE_DELETE(texInfo.Data);
			return handle;
		}
	} else if (IsModel(extention)) {
		ModelInfo modelInfo;
		error = m_ModelLoader->LoadModel(file, modelInfo);
		if (!error) {
			ResourceHandle handle = m_Allocator.AllocModel(modelInfo, m_Allocator.ModelData, filename);
			//clean up model data
			SAFE_DELETE(modelInfo.Materials);

			for (uint32_t i = 0; i < modelInfo.MeshCount; ++i) {
				SAFE_DELETE(modelInfo.Meshes[i].Indices);
				SAFE_DELETE(modelInfo.Meshes[i].Vertices);
			}
			SAFE_DELETE(modelInfo.Meshes);

			handle |= (RT_MODEL << RESOURCE_INDEX_SHIFT);
			m_ResourceCache[hash] = handle;
			return handle;
		}
	} else if (IsShader(extention)) {
		ShaderInfo shaderInfo;
		error = m_ShaderLoader->LoadShaders(file, shaderInfo);
		if (!error) {
			ResourceHandle handle = m_Allocator.AllocShader(shaderInfo, m_Allocator.ShaderData, filename);
			for (uint32_t i = 0; i < shaderInfo.ShaderCount; ++i) {
				free(shaderInfo.Shaders[i].ByteCode);
				free(shaderInfo.Shaders[i].DependenciesHashes);
			}
			free(shaderInfo.Shaders);
			handle |= (RT_SHADER << RESOURCE_INDEX_SHIFT);
			m_ResourceCache[hash] = handle;
			return handle;
		}
	}

	printf("Error loading asset %s\n Error: %s\n", filename, error);
	return -1;
}

void* AssetLoader::GetAsset(ResourceHandle handle, ResourceTypes type) {
	return nullptr;
}

void AssetLoader::LoadStringPool(const char* filename) {
	m_StringPool.DeSerialize(filename);
}

void AssetLoader::SaveStringPool(const char* filename) {
	m_StringPool.Serialize(filename);
}