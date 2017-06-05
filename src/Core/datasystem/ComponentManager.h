#pragma once
#include "ComponentBuffer.h"
#include "../entity/Entity.h"
#include <map>
#define g_ComponentManager ComponentManager::GetInstance()
class ComponentManager {
  public:
	~ComponentManager();
	static ComponentManager& GetInstance();
	void AddComponentType(uint maxCount, size_t size, uint componentID, const char* name = "");
	void CreateComponent(const void* comp, Entity& ent, uint type);
	void RemoveComponent(Entity& ent, uint type);
	void RemoveComponents(Entity& ent);
	int GetBuffer(void** outBuffer, uint type);
	void* GetComponent(const Entity& ent, uint type);
  private:
	ComponentManager();
	void CreateComponentBuffer(uint count, uint componentSize, uint id, std::string name);

	std::map<unsigned int,ComponentBuffer>	m_Buffers;
	int								m_ComponentTypeCount;
};