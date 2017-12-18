#pragma once
#include "Entity.h"
#include <vector>
#define g_EntityManager EntityManager::GetInstance()

struct EntityCache {
	uint64_t ComponentBitMask;
	std::vector<uint32_t> Entities;
};

class EntityManager {
  public:
	~EntityManager();
	static EntityManager& GetInstance();

	Entity& CreateEntity();
	Entity& GetEntity(uint32_t UID);
	void RemoveEntity(Entity& entity);
	void RemoveAllEntities();
	std::vector<Entity>& GetEntityList();
	bool IsCacheDirty(const EntityCache& cache);
  private:
	EntityManager();
	std::vector<Entity> m_Entities;
	uint64_t m_Counter = 0;
	uint64_t m_DirtyComponents = 0;
};