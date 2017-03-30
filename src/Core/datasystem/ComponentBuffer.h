#pragma once
#include <malloc.h>
#include <assert.h>
#include <deque>
#include <string>
typedef  unsigned char Byte;
typedef  unsigned int uint;

class ComponentBuffer {
public:
	ComponentBuffer() {

	}
	~ComponentBuffer() {

	}

	void CreateBuffer(uint count, uint size, std::string name) {
		m_Buffer = (Byte*)malloc(count * size);
		assert(m_Buffer != nullptr);
		memset(m_Buffer, 0, count * size);
		m_Size = count;
		m_ComponentSize = size;
		m_Name = name;
	}
	void ResizeBuffer(uint newSize) {
		m_Buffer = (Byte*)realloc(m_Buffer, newSize);
		assert(m_Buffer != nullptr);
		memset((Byte*)(m_Buffer) + (m_Size * m_ComponentSize), 0, m_Size * m_ComponentSize);
		m_Size *= 2;
	}

	void DestroyBuffer() {
		if (m_Buffer) free(m_Buffer);
	}
	//makes a copy of the content at component
	uint AddComponent(const void* component) {
		if (m_End == m_Size)
			ResizeBuffer(m_ComponentSize * m_Size * 2);
		//find an index, if there is a hole get that instead of end
		uint index;
		if(!m_Holes.empty()){
			index = m_Holes.front();
			m_Holes.pop_front();
		}else {
			index = m_End;
			m_End++;
		}
		//copy data
		memcpy((unsigned char*)(m_Buffer) + index * m_ComponentSize, component, m_ComponentSize);
		m_Count++;
		return index;
	}

	void RemoveComponent(uint index) {
		if (index >= m_End)
			return;
		//reset memory
		memset((Byte*)m_Buffer + index * m_ComponentSize, 0, m_ComponentSize);
		//add to holes queue
		m_Holes.push_front(index);
		m_Count--;
	}

	void* GetComponent(uint index) {
		if (index >= m_Size)
			return nullptr;
		for(auto& hole : m_Holes){
			if(index == hole)
				return nullptr;
		}
		return (unsigned char*)(m_Buffer) + index * m_ComponentSize;
	}
	
	void* GetComponentList() {
		return m_Buffer;
	}

	uint GetListSize() {
		return m_Count;
	}
	std::string& GetName() {
		return m_Name;
	}

private:
	uint m_Size = 0; //sizeof memory allocated
	uint m_End = 0; //end of current list
	uint m_Count = 0; //number of alive components
	uint m_ComponentSize = 0; //size of component in bytes
	void* m_Buffer = nullptr;
	std::deque<uint> m_Holes;
	std::string m_Name;
};