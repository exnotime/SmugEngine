#include "ComponentManager.h"
#include <cmath>
ComponentManager::ComponentManager() {

}

ComponentManager::~ComponentManager() {
	//release all buffers
	for (auto& buffer : m_Buffers) {
		buffer.second.DestroyBuffer();
	}
}

ComponentManager& ComponentManager::GetInstance() {
	static ComponentManager instance;
	return instance;
}

void ComponentManager::AddComponentType(uint maxCount, size_t size, uint componentID, const char* name) {
	CreateComponentBuffer(maxCount, size, componentID, name);
}

void ComponentManager::CreateComponent(const void* comp, Entity& ent, uint type) {
	auto& buffer = m_Buffers.find(type);
	if (buffer != m_Buffers.end()) {
		int index = buffer->second.AddComponent(comp);
		if (ent.Components.size() < m_Buffers.size()) {
			ent.Components.resize(m_Buffers.size());
		}
		ent.ComponentBitfield = ent.ComponentBitfield | type;
		ent.Components[log2(type)] = index;
	} else {
		printf("trying to create component without initializing a buffer\n");
	}
}

void ComponentManager::RemoveComponent(Entity& ent, uint type) {
	auto& buffer = m_Buffers.find(type);
	if (buffer != m_Buffers.end()) {
		buffer->second.RemoveComponent(ent.Components[log2(type)]);
		ent.Components[log2(type)] = 0;
		ent.ComponentBitfield &= ~type;
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

int ComponentManager::GetBuffer(void** outBuffer, uint type) {
	auto buffer = m_Buffers.find(type);

	if (buffer != m_Buffers.end()) {

		*outBuffer = (void*)buffer->second.GetComponentList();
		return buffer->second.GetListSize();
	} else {
		printf("No componentbuffer of such type: %d\n", type);
		*outBuffer = nullptr;
		return -1;
	}
}

void* ComponentManager::GetComponent(const Entity& ent, uint type) {
	auto buffer = m_Buffers.find(type);

	if (buffer != m_Buffers.end()) {
		void* comp = buffer->second.GetComponent(ent.Components[log2(type)]);
		return comp;
	} else {
		printf("Error getting component\n");
		return nullptr;
	}
}

void ComponentManager::CreateComponentBuffer(uint count, uint componentSize, uint id, std::string name) {
	ComponentBuffer buffer;
	buffer.CreateBuffer(count, componentSize, name);
	m_Buffers[id] = buffer;
}