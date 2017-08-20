#pragma once
#include <Graphics/GraphicsEngine.h>
#include <Physics/PhysicsEngine.h>
#include <Core/datasystem/ComponentManager.h>

namespace globals {
extern GraphicsEngine* g_Gfx;
extern PhysicsEngine* g_Physics;
extern ComponentManager* g_Components;
void Clear();
};