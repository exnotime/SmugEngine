#include "AssetLoader.h"
#include <string>
#include <stdio.h>

AssetLoader::AssetLoader(){}

AssetLoader::~AssetLoader(){
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

ResourceHandle AssetLoader::LoadAsset(const char* filename) {
	std::string file(filename);
	std::string extention = file.substr(file.find_last_of('.') + 1);
	char* error = nullptr;
	if (IsTexture(extention)) {
		TextureInfo texInfo;
		error = m_TexLoader.LoadTexture(file, texInfo);
		if (!error) {
			ResourceHandle handle = m_Allocator.AllocTexture(texInfo, m_Allocator.TextureData);
			handle |= (RT_TEXTURE << 32);
			return handle;
		}
	}
	else if (IsModel(extention)) {
		ModelInfo modelInfo;
		error = m_ModelLoader.LoadModel(file, modelInfo);
		if (!error) {
			ResourceHandle handle = m_Allocator.AllocModel(modelInfo, m_Allocator.ModelData);
			//clean up model
			for (int i = 0; i < modelInfo.MaterialCount; ++i) {
				SAFE_DELETE(modelInfo.Materials[i].Albedo.Data);
				SAFE_DELETE(modelInfo.Materials[i].Normal.Data);
				SAFE_DELETE(modelInfo.Materials[i].Roughness.Data);
				SAFE_DELETE(modelInfo.Materials[i].Metal.Data);
			}
			SAFE_DELETE(modelInfo.Materials);

			for (int i = 0; i < modelInfo.MeshCount; ++i) {
				SAFE_DELETE(modelInfo.Meshes[i].Indices);
				SAFE_DELETE(modelInfo.Meshes[i].Vertices);
			}
			SAFE_DELETE(modelInfo.Meshes);

			handle |= (RT_MODEL << 32);
			return handle;
		}
	}

	printf("Error loading asset %s\n Error: %s\n", filename, error);
	return -1;
}