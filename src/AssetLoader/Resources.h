#pragma once
#include <stdint.h>
#include <string>
#include <glm/glm.hpp>
#include "AssetExport.h"

#define RESOURCE_TYPE_MASK 0x00000000ffffffff
#define RESOURCE_INDEX_SHIFT 32
typedef uint64_t ResourceHandle; //first 32 bits say type, second 32 bits say resource index


struct ASSET_DLL TextureInfo {
	uint32_t Width;
	uint32_t Height;
	uint32_t MipCount;
	uint32_t Layers;
	uint32_t Format; //matches ogl/vulkan format
	uint32_t BPP;
	uint32_t LinearSize;
	void* Data = nullptr;
};

struct ASSET_DLL MaterialInfo {
	ResourceHandle Albedo;
	ResourceHandle Normal;
	ResourceHandle Roughness;
	ResourceHandle Metal;
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

enum SHADER_TYPE {
	VERTEX, FRAGMENT, GEOMETRY, EVALUATION, CONTROL, COMPUTE
};

struct ASSET_DLL ShaderInfo {
	SHADER_TYPE Type;
	uint32_t ByteCodeSize;
	void* ByteCode;
};