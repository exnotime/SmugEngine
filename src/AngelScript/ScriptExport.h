#pragma once

#ifdef SCRIPT_EXPORT
#define SCRIPT_DLL __declspec(dllexport)
#else
#define SCRIPT_DLL __declspec(dllimport)
#endif