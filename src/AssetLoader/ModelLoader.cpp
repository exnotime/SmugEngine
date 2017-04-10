#include "ModelLoader.h"
#include "TextureLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <algorithm>

ModelLoader::ModelLoader() {}
ModelLoader::~ModelLoader() {}

char* ModelLoader::LoadModel(const std::string& filename, ModelInfo& model) {
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename, aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals
		| aiProcess_GenUVCoords | aiProcess_JoinIdenticalVertices | aiProcess_Triangulate);
	if (scene) {
		//meshes
		if (scene->HasMeshes()) {
			model.MeshCount = scene->mNumMeshes;
			model.Meshes = new MeshInfo[model.MeshCount];
			for (int m = 0; m < model.MeshCount; ++m) {
				MeshInfo& meshInfo = model.Meshes[m];
				aiMesh* mesh = scene->mMeshes[m];

				meshInfo.Material = mesh->mMaterialIndex;
				meshInfo.VertexCount = mesh->mNumVertices;
				meshInfo.Vertices = new Vertex[meshInfo.VertexCount];
 				for (int v = 0; v < meshInfo.VertexCount; ++v) {
					meshInfo.Vertices[v].Position = glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
					meshInfo.Vertices[v].Normal = glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
					meshInfo.Vertices[v].Tangent = glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
					meshInfo.Vertices[v].TexCoord = glm::vec2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				}
				meshInfo.IndexCount = mesh->mNumFaces * 3;
				meshInfo.Indices = new uint32_t[meshInfo.IndexCount];
				for (int f = 0; f < mesh->mNumFaces; ++f) {
					meshInfo.Indices[f * 3] = mesh->mFaces[f].mIndices[0];
					meshInfo.Indices[f * 3 + 1] = mesh->mFaces[f].mIndices[1];
					meshInfo.Indices[f * 3 + 2] = mesh->mFaces[f].mIndices[2];
				}
			}
		}
		if (scene->HasMaterials()) {
			model.MaterialCount = scene->mNumMaterials;
			model.Materials = new MaterialInfo[model.MaterialCount];
			for (int m = 0; m < scene->mNumMeshes; ++m) {
				aiMaterial* mat = scene->mMaterials[m];
				std::string dir = filename.substr(0, filename.find_last_of('/') + 1);
				aiString path;
				TextureLoader texLoader;
				if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
					mat->GetTexture(aiTextureType_DIFFUSE, 0, &path);
					texLoader.LoadTexture(dir + path.data, model.Materials[m].Albedo);
				}
				if (mat->GetTextureCount(aiTextureType_HEIGHT)) {
					mat->GetTexture(aiTextureType_HEIGHT, 0, &path);
					texLoader.LoadTexture(dir + path.data, model.Materials[m].Normal);
				}
				if (mat->GetTextureCount(aiTextureType_SPECULAR)) {
					mat->GetTexture(aiTextureType_SPECULAR, 0, &path);
					texLoader.LoadTexture(dir + path.data, model.Materials[m].Roughness);
				}
				if (mat->GetTextureCount(aiTextureType_SHININESS)) {
					mat->GetTexture(aiTextureType_SHININESS, 0, &path);
					texLoader.LoadTexture(dir + path.data, model.Materials[m].Metal);
				}
			}
		}
	}
	else {
		return "Assimp error";
	}
	return nullptr;
}