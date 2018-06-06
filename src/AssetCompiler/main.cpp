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

#include <Utility/Filebuffer.h>
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
	for (uint32_t i = 0; i < argc; ++i) {
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
		if (strcmp(argv[i], "-stringpool") == 0 || strcmp(argv[i], "-sp") == 0) {
			opt.ListStringPool = true;
			opt.ListLedger = false;
			opt.Compile = false;
			opt.StringPool = argv[++i];
		}
	}

	return opt;
}

void SanitizeFilePath(std::string& fp) {

	auto Replace = [&]( std::string from, std::string to) {
		size_t p = fp.find(from);
		while (p != std::string::npos) {
			fp.replace(p, from.length(), to);
			p = fp.find(from);
		}
	};

	Replace(".\\", ""); //replace .\\ 
	Replace("\\", "/"); // replace single backslash

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
				std::string s = p.string();
				SanitizeFilePath(s);
				filesOut.push_back(s);
			}
		}
	}
}

void AllocatorResource(const void* data, void* userData, const std::string& filename, const RESOURCE_TYPE type) {
	if (type == RT_MODEL) {
		ModelInfo* info = (ModelInfo*)data;
		int i = 0;
	}
	printf("Allocated Resource %s\n", filename.c_str());
}

int main(const uint32_t argc, const char** argv) {
	Options opts = ParseOptions(argc, argv);

	if (opts.Compile) {
		std::vector<std::string> files;

		std::string modelFolder = opts.AssetFolder + "/models";
		ScanDirForAssets(modelFolder, { ".obj", ".dae" }, files);

		std::string textureFolder = opts.AssetFolder + "/textures";
		ScanDirForAssets(textureFolder, { ".dds" }, files);

		std::string shaderFolder = opts.AssetFolder + "/shaders";
		ScanDirForAssets(shaderFolder, { ".shader" }, files);

		ResourceAllocator allocs;
		allocs.AllocResource = AllocatorResource;
		allocs.UserData = nullptr;
		auto& assetLoader = g_AssetLoader;

		assetLoader.SetResourceAllocator(allocs);
		assetLoader.Init(opts.DataFolder.c_str(), true);

		for (auto& f : files) {
			assetLoader.LoadAsset(f.c_str());
			if (opts.Verbose) {
				printf("Compiled Shader %s\n", f.c_str());
			}
		}
		assetLoader.SaveStringPool("Assets.strings");
		assetLoader.Close();

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
	} else if (opts.ListStringPool) {
		StringPool pool;
		pool.DeSerialize(opts.StringPool);
		pool.Print();
	}

	printf("\nPress any button to quit\n");
	std::getchar();
}