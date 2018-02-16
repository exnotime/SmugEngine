#include "TextureLoader.h"
#include <gli/gli.hpp>
#include <Utility/Hash.h>
#include <Utility/Memory.h>
using namespace smug;

TextureLoader::TextureLoader() {}
TextureLoader::~TextureLoader() {}

char* TextureLoader::LoadTexture(const std::string& filename, TextureInfo& info) {
	gli::texture texture(gli::load(filename));
	if (texture.empty()) {
		return "Loading texture";
	}
	info.Width = texture.extent().x;
	info.Height = texture.extent().y;
	info.Layers = texture.extent().z;
	info.Format = texture.format();
	info.MipCount = uint32_t(texture.levels());
	info.LinearSize = uint32_t(texture.size());
	info.BPP = uint32_t((texture.size(0) * 8) / (texture.extent(0).x * texture.extent(0).y)); //bits for whole mip 0 / number of pixels
	info.Data = malloc(texture.size());
	memcpy(info.Data, texture.data(), texture.size());
	return nullptr;
}

LoadResult TextureLoader::LoadAsset(const char* filename) {
	TextureInfo* info = new TextureInfo();
	char* error = LoadTexture(filename, *info);
	LoadResult res;
	if (error) {
		res.Error = error;
	} else {
		res.Hash = HashString(filename);
		res.Data = info;
		res.Type = RT_TEXTURE;
	}
	return res;
}

void TextureLoader::UnloadAsset(void* asset) {
	TextureInfo* i = (TextureInfo*)asset;
	free(i->Data);
	free(i);
}

void TextureLoader::SerializeAsset(FileBuffer* buffer, LoadResult* asset) {
	//serialize texture
	TextureInfo i = *(TextureInfo*)asset->Data;
	i.Data = (void*)sizeof(TextureInfo);
	buffer->Write(sizeof(TextureInfo), &i, asset->Hash);
	TextureInfo* info = (TextureInfo*)asset->Data;
	buffer->Write(info->LinearSize, info->Data, asset->Hash);
}

DeSerializedResult TextureLoader::DeSerializeAsset(void* assetBuffer) {
	TextureInfo* src = (TextureInfo*)assetBuffer;
	TextureInfo* dst = new TextureInfo();
	*dst = *src;
	dst->Data = malloc(dst->LinearSize);
	memcpy(dst->Data, PointerAdd(src, (size_t)src->Data), src->LinearSize);
	DeSerializedResult res;
	res.Data = dst;
	res.Type = RT_TEXTURE;
	return res;
}

bool TextureLoader::IsExtensionSupported(const char* extension) {
	const char* extensions[] = { "dds", "ktx" };
	for (auto& ext : extensions) {
		if (strcmp(ext, extension) == 0)
			return true;
	}
	return false;
}