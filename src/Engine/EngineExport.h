#pragma once
#if defined(ENGINE_DLL_EXPORT)
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif