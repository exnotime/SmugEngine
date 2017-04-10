#include "TextureLoader.h"
#include <gli/gli.hpp>
TextureLoader::TextureLoader(){}
TextureLoader::~TextureLoader(){}

char* TextureLoader::LoadTexture(const std::string& filename, TextureInfo& info) {
	gli::texture texture(gli::load(filename));
	if (texture.empty()) {
		return "Error: Loading texture";
	}
	info.Width = texture.extent().x;
	info.Height = texture.extent().y;
	info.Layers = texture.extent().z;
	info.Format = texture.format();
	info.MipCount = texture.levels();
	info.Data = malloc(texture.size());
	memcpy(info.Data, texture.data(), texture.size());

	return nullptr;
	
}