#include "ComponentManager.h"
#include <intrin.h>

ComponentManager::ComponentManager() {

}

ComponentManager::~ComponentManager() {
	//release all buffers
	for (auto& buffer : m_Buffers) {
		buffer.DestroyBuffer();
	}
}

void ComponentManager::AddComponentType(uint32_t maxCount, uint32_t size, uint64_t componentID, const char* name) {
	CreateComponentBuffer(maxCount, size, componentID, name);
}

void ComponentManager::CreateComponent(const void* comp, Entity& ent, uint64_t type) {
	uint32_t i = fast_log2(type);
	if (i < m_Buffers.size()) {
		uint32_t componentIndex = m_Buffers[i].AddComponent(comp);
		AddComponentToEntity(ent, type, componentIndex);
	} else {
		printf("trying to create component without initializing a buffer\n");
	}
}

void ComponentManager::RemoveComponent(Entity& ent, uint64_t type) {
	uint32_t i = fast_log2(type);
	if (i < m_Buffers.size()) {
		m_Buffers[i].RemoveComponent(GetEntityComponentIndex(ent, type));
		RemoveComponentfromEntity(ent, type);
	} else {
		printf("trying to remove component without initializing a buffer\n");
	}
}

void ComponentManager::RemoveComponents(Entity& ent) {
	unsigned int compFlag = 1;
	for (int i = 0; i < m_Buffers.size(); i++) {
		if ((ent.ComponentBitfield & compFlag) == compFlag) {
			RemoveComponent(ent, compFlag);
		}
		compFlag = compFlag << 1;
	}
}

int ComponentManager::GetBuffer(void** outBuffer, uint64_t type) {
	uint32_t i = fast_log2(type);
	if (i < m_Buffers.size()) {
		*outBuffer = (void*)m_Buffers[i].GetComponentList();
		return m_Buffers[i].GetListSize();
	} else {
		printf("No componentbuffer of such type: %d\n", type);
		*outBuffer = nullptr;
		return -1;
	}
}

void* ComponentManager::GetComponent(const Entity& ent, uint64_t type) {
	uint32_t i = fast_log2(type);
	if (i < m_Buffers.size()) {
		void* comp = m_Buffers[i].GetComponent(GetEntityComponentIndex(ent, type));
		return comp;
	} else {
		printf("Error getting component\n");
		return nullptr;
	}
}

void ComponentManager::CreateComponentBuffer(uint32_t count, uint32_t componentSize, uint64_t id, std::string name) {
	ComponentBuffer buffer;
	buffer.CreateBuffer(count, componentSize, name);
	uint32_t bit_index = fast_log2(id);
	if (m_Buffers.size() < bit_index + 1) {
		m_Buffers.resize(bit_index + 1);
	}
	m_Buffers[bit_index] = buffer;
}