#pragma once
#include "ComponentBuffer.h"
#include "../entity/Entity.h"
#include <vector>
class ComponentManager {
  public:
	ComponentManager();
	~ComponentManager();
	void AddComponentType(uint maxCount, uint size, uint componentID, const char* name = "");
	void CreateComponent(const void* comp, Entity& ent, uint type);
	void RemoveComponent(Entity& ent, uint type);
	void RemoveComponents(Entity& ent);
	int GetBuffer(void** outBuffer, uint type);
	void* GetComponent(const Entity& ent, uint type);
  private:
	void CreateComponentBuffer(uint count, uint componentSize, uint id, std::string name);

	std::vector<ComponentBuffer>	m_Buffers;
	int								m_ComponentTypeCount;
};