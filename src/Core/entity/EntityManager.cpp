#include "EntityManager.h"
#include "../datasystem/ComponentManager.h"
EntityManager::EntityManager(){}

EntityManager::~EntityManager(){}

EntityManager& EntityManager::GetInstance() {
	static EntityManager instance;
	return instance;
}

Entity& EntityManager::CreateEntity() {
	Entity e;
	e.ComponentBitfield = 0;
	e.Components.reserve(ENTITY_DEFAULT_COMPONENT_COUNT);
	e.UID = m_Counter++;
	m_Entities.push_back(e);
	return m_Entities.back();
}

void EntityManager::RemoveEntity(Entity& entity) {
	//potentially slow
	for (int i = 0; i < m_Entities.size(); i++) {
		if (m_Entities[i].UID == entity.UID) {
			g_ComponentManager.RemoveComponents(m_Entities[i]);
			m_Entities.erase( m_Entities.begin() + i);
			return;
		}
	}
}

void EntityManager::RemoveAllEntities() {
	for (int i = 0; i < m_Entities.size(); i++) {
		g_ComponentManager.RemoveComponents(m_Entities[i]);
	}
	m_Entities.clear();
}

std::vector<Entity>& EntityManager::GetEntityList() {
	return m_Entities;
}