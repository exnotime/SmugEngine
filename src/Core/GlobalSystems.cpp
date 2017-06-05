#include "GlobalSystems.h"

GraphicsEngine* globals::g_Gfx = nullptr;
PhysicsEngine* globals::g_Physics = nullptr;

void globals::Clear() {
	delete g_Gfx;
	g_Gfx = nullptr;
	delete g_Physics;
	g_Physics = nullptr;
}