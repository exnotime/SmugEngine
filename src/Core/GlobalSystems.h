#pragma once
#include <Graphics/GraphicsEngine.h>
#include <Physics/PhysicsEngine.h>
#include <Core/datasystem/ComponentManager.h>
#include <Core/entity/EntityManager.h>

namespace smug {
namespace globals {
extern GraphicsEngine* g_Gfx;
extern PhysicsEngine* g_Physics;
extern ComponentManager* g_Components;
extern EntityManager* g_EntityManager;
void Clear();
}
}