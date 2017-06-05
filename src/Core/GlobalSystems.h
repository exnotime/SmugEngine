#pragma once
#include <Graphics/GraphicsEngine.h>
#include <Physics/PhysicsEngine.h>
namespace globals {
extern GraphicsEngine* g_Gfx;
extern PhysicsEngine* g_Physics;
void Clear();
};