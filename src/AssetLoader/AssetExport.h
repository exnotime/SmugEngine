#pragma once

#ifdef ASSET_SHARED_DLL
	#ifdef ASSET_EXPORT
		#define ASSET_DLL __declspec(dllexport)
	#else
		#define ASSET_DLL __declspec(dllimport)
	#endif
#else
	#define ASSET_DLL
#endif
