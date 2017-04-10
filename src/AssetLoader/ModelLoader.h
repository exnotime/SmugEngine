#pragma once
#include <string>
#include "Resources.h"
#include "AssetExport.h"

class ASSET_DLL ModelLoader {
public:
	ModelLoader();
	~ModelLoader();
	char* LoadModel(const std::string& filename, ModelInfo& info);
};