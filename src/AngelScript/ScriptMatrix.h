#pragma once
#include <glm/glm.hpp>
#include <new>

namespace smug {
struct Mat4x4 {
	Mat4x4();
	~Mat4x4() {}
	glm::vec4 operator*(const glm::vec4& v)const;
	glm::vec4& operator[](const unsigned int);
	glm::mat4x4 m_Mat;
};

struct Mat3x4 {
	Mat3x4();
	~Mat3x4() {}
	glm::vec4 operator*(const glm::vec4& v) const;
	glm::vec4& operator[](const unsigned int);
	glm::mat3x4 m_Mat;
};

struct Mat3x3 {
	Mat3x3();
	~Mat3x3() {}
	glm::vec3 operator*(const glm::vec3& v)const;
	glm::vec3& operator[](const unsigned int);
	glm::mat3x3 m_Mat;
};


static void ConstructMat4x4(void* mem) {
	new(mem) Mat4x4();
}
static void DestructMat4x4(void* mem) {
	((Mat4x4*)mem)->~Mat4x4();
}

static void ConstructMat3x4(void* mem) {
	new(mem) Mat3x4();
}
static void DestructMat3x4(void* mem) {
	((Mat3x4*)mem)->~Mat3x4();
}

static void ConstructMat3x3(void* mem) {
	new(mem) Mat3x3();
}
static void DestructMat3x3(void* mem) {
	((Mat3x3*)mem)->~Mat3x3();
}
}

