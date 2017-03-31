#include "GlobalSystems.h"

GraphicsEngine* globals::g_Gfx = nullptr;

void globals::Clear() {
	delete g_Gfx; g_Gfx = nullptr;
}