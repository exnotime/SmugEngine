#include "GlobalSystems.h"
using namespace smug;

GraphicsEngine* globals::g_Gfx = nullptr;
PhysicsEngine* globals::g_Physics = nullptr;
ComponentManager* globals::g_Components = nullptr;
EntityManager* globals::g_EntityManager = nullptr;

void globals::Clear() {
	delete g_Gfx;
	g_Gfx = nullptr;
	delete g_Physics;
	g_Physics = nullptr;
	delete g_Components;
	g_Components = nullptr;
	delete g_EntityManager;
	g_EntityManager = nullptr;
}