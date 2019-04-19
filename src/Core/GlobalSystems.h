#pragma once


namespace smug {
	class GraphicsEngine;
	class PhysicsEngine;
	class ComponentManager;
	class EntityManager;

	namespace globals {
		extern GraphicsEngine* g_Gfx;
		extern PhysicsEngine* g_Physics;
		extern ComponentManager* g_Components;
		extern EntityManager* g_EntityManager;
		void Clear();
	}
}