#pragma once
#include <string>
#include "Resources.h"
#include "LoaderInterface.h"

namespace smug {
	//Textureloader uses GLI to load dds and ktx
	class TextureLoader : public LoaderInterface {
	public:
		TextureLoader();
		~TextureLoader();
		char* LoadTexture(const std::string& filename, TextureInfo& info);

		virtual LoadResult LoadAsset(const char* filename) override;
		virtual void UnloadAsset(void* asset) override;
		virtual void SerializeAsset(FileBuffer* buffer, LoadResult* asset) override;
		virtual DeSerializedResult DeSerializeAsset(void* assetBuffer) override;
		virtual bool IsExtensionSupported(const char* extension) override;
	};
}