#pragma once

#ifdef GRAPHICS_EXPORT
#define GFX_DLL __declspec(dllexport)
#else
#define GFX_DLL __declspec(dllimport)
#endif
