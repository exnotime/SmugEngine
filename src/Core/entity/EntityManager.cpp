#include "EntityManager.h"
#include <Core/datasystem/ComponentManager.h>
#include <Core/GlobalSystems.h>

using namespace smug;
EntityManager::EntityManager() {}

EntityManager::~EntityManager() {}

Entity& EntityManager::CreateEntity() {
	Entity e;
	e.ComponentBitfield = 0;
	e.UID = m_Counter++;
	m_Entities.push_back(e);
	return m_Entities.back();
}

Entity& EntityManager::GetEntity(uint32_t UID) {
	return m_Entities[UID];
}

void EntityManager::RemoveEntity(Entity& entity) {
	//potentially slow
	for (int i = 0; i < m_Entities.size(); i++) {
		if (m_Entities[i].UID == entity.UID) {
			m_DirtyComponents |= entity.ComponentBitfield;
			globals::g_Components->RemoveComponents(m_Entities[i]);
			m_Entities.erase( m_Entities.begin() + i);
			return;
		}
	}
}

void EntityManager::RemoveAllEntities() {
	for (int i = 0; i < m_Entities.size(); i++) {
		globals::g_Components->RemoveComponents(m_Entities[i]);
	}
	m_Entities.clear();
}

eastl::vector<Entity>& EntityManager::GetEntityList() {
	return m_Entities;
}

bool EntityManager::IsCacheDirty(const EntityCache& cache) {
	return (cache.ComponentBitMask & m_DirtyComponents) == cache.ComponentBitMask;
}