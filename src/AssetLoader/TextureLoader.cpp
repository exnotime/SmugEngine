#include "TextureLoader.h"
#include <gli/gli.hpp>
TextureLoader::TextureLoader() {}
TextureLoader::~TextureLoader() {}

char* TextureLoader::LoadTexture(const std::string& filename, TextureInfo& info) {
	gli::texture texture(gli::load(filename));
	if (texture.empty()) {
		return "Error: Loading texture";
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