#pragma once
#include <stdint.h>
namespace smug {
struct TextureInfo {
	unsigned Width;
	unsigned Height;
	unsigned MipCount;
	unsigned Layers;
	unsigned Format; //matches vulkan format
	void* Data;
};

struct MaterialInfo {
	unsigned TextureCount;
	TextureInfo* Textures;
};

struct MeshInfo {
	unsigned VertexCount;
	unsigned IndexCount;
	unsigned Material;
};

struct ModelInfo {
	unsigned MeshCount;
	MeshInfo* Meshes;
	unsigned MaterialCount;
	MaterialInfo* Materials;
};

typedef uint64_t ResourceHandle; //first 32 bits say type, second 32 bits say resource index
}