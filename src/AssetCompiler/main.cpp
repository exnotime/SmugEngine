#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#ifdef _WIN32
#include <experimental/filesystem> // C++-standard header file name  
#include <filesystem> // Microsoft-specific implementation header file name  
using namespace std::experimental::filesystem::v1;
#endif

#include "Filebuffer.h"
#include <AssetLoader/AssetLoader.h>

struct Options {
	std::string AssetFolder = "./assets"; //source assets
	std::string DataFolder = "./data"; //destination of compiled data
	bool Compile = true; //default option is to compile data
	bool ListLedger = false; //list what is in the ledger file
	std::string Ledger = ""; //no default value
	bool ListStringPool = false; //List all string stored in a string folder
	std::string StringPool = ""; //file to keep strings in
	bool Verbose = false; //print what files are compiled
	bool MidFlush = true; //Write out every file to disk directly after compile
};

Options ParseOptions(const uint32_t argc, const char** argv) {
	Options opt;
	for (uint32_t i = 0; i < argc; ++i){
		if (strcmp(argv[i], "-assetfolder") == 0 || strcmp(argv[i], "-af") == 0) {
			opt.AssetFolder = argv[++i];
		}
		if (strcmp(argv[i], "-datafolder") == 0 || strcmp(argv[i], "-df") == 0) {
			opt.DataFolder = argv[++i];
		}
		if (strcmp(argv[i], "-ledger") == 0 || strcmp(argv[i], "-l") == 0) {
			opt.ListLedger = true;
			opt.Compile = false;
			opt.Ledger = argv[++i];
		}
		if (strcmp(argv[i], "-verbose") == 0 || strcmp(argv[i], "-v") == 0) {
			opt.Verbose = true;
		}
		if (strcmp(argv[i], "-no-mid-flush") == 0 || strcmp(argv[i], "-nmf") == 0) {
			opt.MidFlush = false;
		}
	}

	return opt;
}

void ScanDirForAssets(const std::string& dirName, const std::vector<std::string>& fileExtentions, std::vector<std::string>& filesOut) {

	auto dirit = directory_iterator(dirName);
	std::vector<path> files;
	for (auto& p : dirit) {
		if (is_directory(p.path()))
			ScanDirForAssets(p.path().string(), fileExtentions, filesOut);
		else
			files.push_back(p);
	}

	for (auto& p : files) {
		for (auto& ext : fileExtentions) {
			if (p.extension() == ext) {
				filesOut.push_back(p.string());
			}
		}
	}
}

ResourceHandle AllocModel(const ModelInfo& info, void* userData, const std::string& filename) {
	FileBuffer* modelFile = (FileBuffer*)userData;
	//serialize model
	size_t meshDataOffset = sizeof(ModelInfo) + info.MeshCount * sizeof(MeshInfo) + info.MaterialCount * sizeof(MaterialInfo);
	std::vector<MeshInfo> meshes;
	size_t offset = meshDataOffset;
	for (uint32_t m = 0; m < info.MeshCount; ++m) {
		MeshInfo mesh;
		mesh = info.Meshes[m];
		mesh.Vertices = (Vertex*)offset;
		offset += sizeof(Vertex) * mesh.VertexCount;
		mesh.Indices = (uint32_t*)offset;
		offset += mesh.IndexCount * sizeof(uint32_t);
		meshes.push_back(mesh);
	}

	ModelInfo model;
	model = info;
	model.Meshes = (MeshInfo*)sizeof(ModelInfo);
	model.Materials =  (MaterialInfo*)(model.Meshes + sizeof(MaterialInfo) * info.MaterialCount);

	

	modelFile->Write(sizeof(ModelInfo), &model, filename);
	modelFile->Write(sizeof(MeshInfo) * meshes.size(), meshes.data(), filename);
	modelFile->Write(sizeof(MaterialInfo) * info.MaterialCount, info.Materials, filename);
	for (uint32_t m = 0; m < info.MeshCount; ++m) {
		 modelFile->Write(info.Meshes[m].VertexCount * sizeof(Vertex), info.Meshes[m].Vertices, filename);
		 modelFile->Write(info.Meshes[m].IndexCount * sizeof(uint32_t), info.Meshes[m].Indices, filename);
	}

	return 0;
}


ResourceHandle AllocTexture(const TextureInfo& info, void* userData, const std::string& filename) {
	FileBuffer* textureFile = (FileBuffer*)userData;
	//serialize texture
	TextureInfo i = info;
	i.Data = (void*)sizeof(TextureInfo);
	textureFile->Write(sizeof(TextureInfo), &i, filename);
	textureFile->Write(info.LinearSize, info.Data, filename);
	return 0;
}

ResourceHandle AllocShader(const ShaderInfo& info, void* userData, const std::string& filename) {
	FileBuffer* shaderFile = (FileBuffer*)userData;
	//serialize shader
	ShaderInfo si = info;
	uint32_t offset = sizeof(ShaderInfo) + sizeof(ShaderByteCode) * si.ShaderCount;
	std::vector<ShaderByteCode> byteCodes;
	for (uint32_t i = 0; i < si.ShaderCount; ++i) {
		ShaderByteCode bc = si.Shaders[i];
		bc.ByteCode = (void*)offset;
		offset += si.Shaders[i].ByteCodeSize;
		bc.DependenciesHashes = (uint32_t*)offset;
		offset += si.Shaders[i].DependencyCount * sizeof(uint32_t);
		byteCodes.push_back(bc);
	}

	shaderFile->Write(sizeof(ShaderInfo), (void*)&si, filename);
	shaderFile->Write(sizeof(ShaderByteCode) * byteCodes.size(), (void*)byteCodes.data(), filename);
	for (uint32_t i = 0; i < info.ShaderCount; ++i) {
		shaderFile->Write(info.Shaders[i].ByteCodeSize, (void*)info.Shaders[i].ByteCode, filename);
		shaderFile->Write(sizeof(uint32_t) * info.Shaders[i].DependencyCount, (void*)info.Shaders[i].DependenciesHashes, filename);
	}
	return 0;
}

int main(const uint32_t argc, const char** argv) {
	Options opts = ParseOptions(argc, argv);

	if (opts.Compile) {
		std::string modelFolder = opts.AssetFolder + "/models";
		std::vector<std::string> modelFiles;
		ScanDirForAssets(modelFolder, { ".obj", ".dae" }, modelFiles);

		std::string textureFolder = opts.AssetFolder + "/textures";
		std::vector<std::string> textureFiles;
		ScanDirForAssets(textureFolder, { ".dds" }, textureFiles);

		std::string shaderFolder = opts.AssetFolder + "/shaders";
		std::vector<std::string> shaderFiles;
		ScanDirForAssets(shaderFolder, { ".shader" }, shaderFiles);

		FileBuffer modelFile;
		FileBuffer textureFile;
		FileBuffer shaderFile;

		modelFile.Open((opts.DataFolder + "/Models.bank").c_str(), (opts.DataFolder + "/Models.ledger").c_str());
		textureFile.Open((opts.DataFolder + "/Textures.bank").c_str(), (opts.DataFolder + "/Textures.ledger").c_str());
		shaderFile.Open((opts.DataFolder + "/Shaders.bank").c_str(), (opts.DataFolder + "/Shaders.ledger").c_str());

		ResourceAllocator allocs;
		allocs.AllocModel = AllocModel;
		allocs.ModelData = (void*)&modelFile;
		allocs.AllocTexture = AllocTexture;
		allocs.TextureData = (void*)&textureFile;
		allocs.AllocShader = AllocShader;
		allocs.ShaderData = (void*)&shaderFile;

		g_AssetLoader.SetResourceAllocator(allocs);

		for (auto& mf : modelFiles) {
			g_AssetLoader.LoadAsset(mf.c_str());
			if (opts.Verbose) {
				printf("Compiled Model %s\n", mf.c_str());
			}
			if (opts.MidFlush) {
				modelFile.Flush();
			}
		}
		for (auto& tf : textureFiles) {
			g_AssetLoader.LoadAsset(tf.c_str());
			if (opts.Verbose) {
				printf("Compiled Texture %s\n", tf.c_str());
			}
			if (opts.MidFlush) {
				textureFile.Flush();
			}
		}

		for (auto& sf : shaderFiles) {
			g_AssetLoader.LoadAsset(sf.c_str());
			if (opts.Verbose) {
				printf("Compiled Shader %s\n", sf.c_str());
			}
			if (opts.MidFlush) {
				shaderFile.Flush();
			}
		}

		modelFile.Close();
		textureFile.Close();
		shaderFile.Close();

	} else if (opts.ListLedger) {
		FILE* f = fopen(opts.Ledger.c_str(), "rb");
		if (f) {
			printf("Listing content of ledger %s\n", opts.Ledger.c_str());
			LedgerEntry entry;
			while (fread(&entry, sizeof(LedgerEntry), 1, f) == 1) {
				if (feof(f))
					break;
				printf("File %zx of size %zu is at offset %zu\n", entry.Hash, entry.Size, entry.Offset);
			}
			fclose(f);
		} else {
			printf("Error opening Ledger %s\n", opts.Ledger.c_str());
		}
	}

	printf("\nPress any button to quit\n");
	std::getchar();
}