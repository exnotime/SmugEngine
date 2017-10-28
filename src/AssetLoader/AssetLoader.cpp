#include "AssetLoader.h"
#include <string>
#include <stdio.h>
#include "TextureLoader.h"
#include "ModelLoader.h"
#include "ShaderLoader.h"
#include <Utility/Hash.h>
#define SAFE_DELETE(x) if(x) delete x

AssetLoader::AssetLoader() {
	m_ResourceCache.reserve(1);
}

AssetLoader::~AssetLoader() {
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

	for (auto& l : m_Loaders) {
		l.buffer->Close();
		delete l.buffer;
		delete l.loader;
	}
}

ResourceHandle CreateHandle(uint32_t hash, RESOURCE_TYPE type) {
	ResourceHandle h = type;
	h << RESOURCE_HASH_SHIFT;
	h |= hash;
	return h;
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
					ResourceHandle h = CreateHandle(hash, asset.Type);
					m_ResourceCache[hash] = h;
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

			ResourceHandle h = CreateHandle(hash, r.Type);
			m_ResourceCache[hash] = h;
			return h;
		}
	}

	printf("No loader for extention %s\n", extention.c_str());
	return -1;
}

void AssetLoader::LoadStringPool(const char* filename) {
	m_StringPool.DeSerialize(filename);
}

void AssetLoader::SaveStringPool(const char* filename) {
	m_StringPool.Serialize(filename);
}