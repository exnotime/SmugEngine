#include <stdint.h>
#include <stdio.h>
#include <string>
#ifdef _WIN32
#include <experimental/filesystem> // C++-standard header file name  
#include <filesystem> // Microsoft-specific implementation header file name  
using namespace std::experimental::filesystem::v1;
#endif

struct Options {
	std::string AssetFolder = "./assets"; //source assets
	std::string DataFolder = "./data"; //destination of compiled data
};

Options ParseOptions(const uint32_t argc, const char** argv) {
	Options opt;
	for (uint32_t i = 0; i < argc; ++i){
		if (strcmp(argv[i], "--assetfolder") == 0) {
			opt.AssetFolder = argv[++i];
		}
		if (strcmp(argv[i], "--datafolder") == 0) {
			opt.DataFolder = argv[++i];
		}
	}

	return opt;
}

int main(const uint32_t argc, const char** argv) {
	Options opts = ParseOptions(argc, argv);
	char buffer[_MAX_PATH];
	_fullpath(buffer, opts.AssetFolder.c_str(), _MAX_PATH);
	path assetRoot(buffer);
	printf("Asset root is %s\n", assetRoot.root_path().c_str());

	while (true);
}