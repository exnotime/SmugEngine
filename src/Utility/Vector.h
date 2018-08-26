#pragma once
#include <stdint.h>
#include <new>
#include <assert.h>
/*
namespace smug {
template<class T>
class Vector {
  public:
	Vector() {
		Capacity = 1;
		Size = 0;
		Data = (uint8_t*)malloc(sizeof(T));
		assert(Data != nullptr);
	}
	~Vector() {
		for (uint64_t i = 0; i < Size; ++i) {
			((T*)(Data + i * sizeof(T)))->~T();
		}
		free(Data);
		Data = nullptr;
	}

	//functions
	void push_back(const T& t) {
		if (Size + 1 > Capacity)
			reserve(Capacity * 2 + 8);
		T* q = new(Data + sizeof(T) * Size)T(t);
		Size++;
	}

	T* data() const {
		return (T*)Data;
	}

	//will leave garbage in old memory
	void clear() {
		Size = 0;
	}

	void reserve(uint64_t s) {
		if (s == Capacity)
			return; //job well done
		if (s < Capacity) {
			//shrink
			uint8_t* temp = (uint8_t*)malloc(sizeof(T) * s);
			assert(temp != nullptr);
			memcpy(temp, Data, sizeof(T) * s);
			free(Data);
			Data = temp;
			Size = s;
		}
		if (s > Capacity) {
			//Grow
			Data = (uint8_t*)realloc(Data, sizeof(T) * s);
			assert(Data != nullptr);
		}
		Capacity = s;
	}

	void resize(uint64_t s) {
		reserve(s);
		Size = s;
	}

	uint64_t size() const {
		return Size;
	}

	void erase(const uint32_t i) {
		if (i == Size - 1) {
			//last object just shrink size
			Size--;
			return;
		} else if (i < Size) {
			//somewhere in the middle move every thing after it one step to the left
			memcpy(Data + sizeof(T) * (i - 1), Data + sizeof(T) * (i + 1), sizeof(T) * (Size - i));
			Size--;
			return;
		}
	}

	void insert(const uint32_t i, const T& t) {
		if (i > Size) {
			return;
		}
		if (i == Size) {
			push_back(t);
		}
		if (Size >= Capacity) {
			reserve(Size * 2 + 8);
		}
		memcpy(Data + sizeof(T) * (i + 1), Data + sizeof(T) * i, sizeof(T) * (Size - i));
		new(Data + sizeof(T) * Size)T(t);
	}

	//operators
	T& operator [](const int i) {
		return ((T*)Data)[i];
	}

	const T& operator [](const int i) const {
		return ((T*)Data)[i];
	}

	void operator =(Vector<T>& v) {
		if (v.size() > Size) {
			resize(v.size());
		}
		memcpy(Data, v.data(), sizeof(T) * v.size());
	}

	void operator += (Vector<T>& v) {
		if (Size + v.size() > Capacity) {
			reserve(Size + v.size());
		}
		memcpy(Data + sizeof(T) * Size, v.data(), sizeof(T) * v.size());
		Size += v.size();
	}

	void operator += (const Vector<T>& v) {
		if (Size + v.size() > Capacity) {
			reserve(Size + v.size());
		}
		memcpy(Data + sizeof(T) * Size, v.data(), sizeof(T) * v.size());
		Size += v.size();
	}

  private:
	uint64_t Capacity;
	uint64_t Size;
	uint8_t* Data;
};
}
*/