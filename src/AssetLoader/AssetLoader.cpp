#include "AssetLoader.h"
#include <string>
#include <stdio.h>
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include <Utility/Hash.h>
#define SAFE_DELETE(x) if(x) delete x
using namespace smug;
AssetLoader::AssetLoader() {
	m_ResourceCache.reserve(1);
}

AssetLoader::~AssetLoader() {
	//Close();
}

AssetLoader& AssetLoader::GetInstance() {
	static AssetLoader instance;
	return instance;
}

void AssetLoader::SetResourceAllocator(ResourceAllocator allocator) {
	m_Allocator = allocator;
}

void AssetLoader::Init(const char* dataFolder, bool isCompiler) {
	m_IsCompiler = isCompiler;

	if (!dataFolder) {
		return;
	}

	//init loaders
	ResourceLoader shaderLoader;
	shaderLoader.buffer = new FileBuffer();
	shaderLoader.loader = new ShaderLoader();
	shaderLoader.LedgerFile = dataFolder + std::string("/Shaders.ledger");
	shaderLoader.BankFile = dataFolder + std::string("/Shaders.bank");
	m_Loaders.push_back(shaderLoader);

	ResourceLoader textureLoader;
	textureLoader.buffer = new FileBuffer();
	textureLoader.loader = new TextureLoader();
	textureLoader.LedgerFile = dataFolder + std::string("/Textures.ledger");
	textureLoader.BankFile = dataFolder + std::string("/Textures.bank");
	m_Loaders.push_back(textureLoader);

	ResourceLoader modelLoader;
	modelLoader.buffer = new FileBuffer();
	modelLoader.loader = new ModelLoader();
	modelLoader.LedgerFile = dataFolder + std::string("/Models.ledger");
	modelLoader.BankFile = dataFolder + std::string("/Models.bank");
	m_Loaders.push_back(modelLoader);

	if (m_IsCompiler) {
		for (auto& l : m_Loaders) {
			l.buffer->OpenForWriting(l.BankFile.c_str(), l.LedgerFile.c_str());
		}
	} else {
		for (auto& l : m_Loaders) {
			l.buffer->OpenForReading(l.BankFile.c_str(), l.LedgerFile.c_str());
		}
	}

}

void AssetLoader::Close() {

	UnloadAllAssets();
	for (auto& l : m_Loaders) {
		l.buffer->Close();
		delete l.buffer;
		delete l.loader;
	}
}

int ExtractType(ResourceHandle handle) {
	return handle >> RESOURCE_HASH_SHIFT;
}

ResourceHandle AssetLoader::LoadAsset(const char* filename) {
	ResourceHash hash = HashString(filename);

	if (m_ResourceCache.find(hash) != m_ResourceCache.end()) {
		return m_ResourceCache.find(hash)->second;
	}

	m_StringPool.AddToPool(hash, filename);

	const std::string file(filename);
	const std::string extention = file.substr(file.find_last_of('.') + 1);

	for (auto& l : m_Loaders) {
		if (l.loader->IsExtensionSupported(extention.c_str())) {
			if (!m_IsCompiler) {
				void* assetBuffer = l.buffer->LoadFile(hash);
				if (assetBuffer) {
					DeSerializedResult asset = l.loader->DeSerializeAsset(assetBuffer);
					m_Allocator.AllocResource(asset.Data, m_Allocator.UserData, filename, asset.Type);
					l.loader->UnloadAsset(asset.Data);
					ResourceHandle h = CreateHandle(hash, asset.Type);
					m_ResourceCache[hash] = h;
					m_ResourceData[h] = assetBuffer;
					return h;
				}
			}

			LoadResult r = l.loader->LoadAsset(filename);

			if (!r.Data && r.Error.length() > 0) {
				printf("Error loading asset %s\n Error: %s\n", filename, r.Error.c_str());
				return -1;
			}

			if (m_IsCompiler) {

				l.loader->SerializeAsset(l.buffer, &r);
				l.buffer->Flush();
			}
			m_Allocator.AllocResource(r.Data, m_Allocator.UserData, filename, r.Type);
			ResourceHandle h = CreateHandle(hash, r.Type);
			m_ResourceCache[hash] = h;
			m_ResourceData[h] = r.Data;
			m_FilenameCache[h] = std::string(filename);
			return h;
		}
	}

	printf("No loader for extention %s\n", extention.c_str());
	return -1;
}

ResourceHandle AssetLoader::LoadGeneratedModel(ModelInfo& modelInfo, const char* name) {
	m_Allocator.AllocResource(&modelInfo, m_Allocator.UserData, name, RT_MODEL);
	uint32_t hash = HashString(name);
	 ResourceHandle h = CreateHandle(hash, RT_MODEL);
	 m_ResourceCache[hash] = h;
	 m_ResourceData[h] = nullptr;
	return h;
}

void AssetLoader::UpdateModel(ResourceHandle handle, ModelInfo& modelInfo) {
	m_Allocator.ReAllocResource(&modelInfo, m_Allocator.UserData, handle);
}

ResourceHandle AssetLoader::LoadAsset(uint32_t hash) {
	std::string name = m_StringPool.GetString(hash);
	if (!name.empty()) {
		return LoadAsset(name.c_str());
	}
	return RESOURCE_INVALID;
}

void AssetLoader::UnloadAsset(ResourceHandle h) {
	int type = ExtractType(h);
	for (auto& loader : m_Loaders) {
		if (loader.loader->IsTypeSupported(type)) {
			auto data = m_ResourceData.find(h);
			if (data != m_ResourceData.end()) {
				if (data->second) {
					loader.loader->UnloadAsset(data->second);
				}
			}
			m_Allocator.DeAllocResource(h, m_Allocator.UserData);
		}
	}
	m_ResourceData.erase(h);
}

void AssetLoader::UnloadAllAssets() {
	for (auto& h : m_ResourceCache) {
		UnloadAsset(h.second);
	}
}

std::string AssetLoader::GetFilenameFromCache(ResourceHandle handle) {
	auto& f = m_FilenameCache.find(handle);
	if (f != m_FilenameCache.end()) {
		return f->second;
	}
	return "";
}

void AssetLoader::LoadStringPool(const char* filename) {
	m_StringPool.DeSerialize(filename);
}

void AssetLoader::SaveStringPool(const char* filename) {
	m_StringPool.Serialize(filename);
}