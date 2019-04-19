#pragma once
#include "ComponentBuffer.h"
#include "../entity/Entity.h"
#include <EASTL/vector.h>

namespace smug {
class ComponentManager {
  public:
	ComponentManager();
	~ComponentManager();
	void AddComponentType(uint32_t maxCount, uint32_t size, uint64_t componentID, const char* name = "");
	void CreateComponent(const void* comp, Entity& ent, uint64_t type);
	void RemoveComponent(Entity& ent, uint64_t type);
	void RemoveComponents(Entity& ent);
	int GetBuffer(void** outBuffer, uint64_t type);
	void* GetComponent(const Entity& ent, uint64_t type);
  private:
	void CreateComponentBuffer(uint32_t count, uint32_t componentSize, uint64_t id, eastl::string name);

	eastl::vector<ComponentBuffer>	m_Buffers;
	int								m_ComponentTypeCount;
};
}