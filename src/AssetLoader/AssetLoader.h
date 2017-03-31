#pragma once

#ifdef ASSET_EXPORT
#define ASSET_DLL __declspec(dllexport)
#else
#define ASSET_DLL __declspec(dllimport)
#endif

class ASSET_DLL AssetLoader {
public:
	AssetLoader();
	~AssetLoader();
	void Init();
};