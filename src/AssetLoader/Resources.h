#pragma once
#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include "AssetExport.h"
typedef uint64_t ResourceHandle; //first 32 bits say type, second 32 bits say resource index

struct ASSET_DLL TextureInfo {
	uint32_t Width;
	uint32_t Height;
	uint32_t MipCount;
	uint32_t Layers;
	uint32_t Format; //matches ogl/vulkan format
	void* Data = nullptr;
};

struct ASSET_DLL MaterialInfo {
	TextureInfo Albedo;
	TextureInfo Normal;
	TextureInfo Roughness;
	TextureInfo Metal;
};

struct ASSET_DLL Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec3 Tangent;
	glm::vec2 TexCoord;
};

struct ASSET_DLL MeshInfo {
	uint32_t VertexCount;
	Vertex* Vertices = nullptr;
	uint32_t IndexCount;
	uint32_t* Indices = nullptr;
	uint32_t Material;
};

struct ASSET_DLL ModelInfo {
	uint32_t MeshCount;
	MeshInfo* Meshes = nullptr;
	uint32_t MaterialCount;
	MaterialInfo* Materials = nullptr;
};