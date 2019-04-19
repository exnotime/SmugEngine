#pragma once
#include "Entity.h"
#include <EASTL/vector.h>
namespace smug {
struct EntityCache {
	uint64_t ComponentBitMask;
	eastl::vector<uint32_t> Entities;
};

class EntityManager {
  public:
	EntityManager();
	~EntityManager();

	Entity& CreateEntity();
	Entity& GetEntity(uint32_t UID);
	void RemoveEntity(Entity& entity);
	void RemoveAllEntities();
	eastl::vector<Entity>& GetEntityList();
	bool IsCacheDirty(const EntityCache& cache);
  private:

	eastl::vector<Entity> m_Entities;
	uint32_t m_Counter = 0;
	uint64_t m_DirtyComponents = 0;
};
}