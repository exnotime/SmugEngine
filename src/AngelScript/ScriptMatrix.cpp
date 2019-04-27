#include "ScriptMatrix.h"

using namespace smug;
//Mat4x4
Mat4x4::Mat4x4() {
	m_Mat = glm::mat4x4(1);
}
glm::vec4 Mat4x4::operator*(const glm::vec4 & v)const {
	return m_Mat * v;
}
glm::vec4& Mat4x4::operator[](const unsigned int i) {
	return m_Mat[i];
}
//mat3x4
Mat3x4::Mat3x4() {
	m_Mat = glm::mat3x4(1);
}
glm::vec4 Mat3x4::operator*(const glm::vec4 & v)const {
	return m_Mat * v;
}
glm::vec4& Mat3x4::operator[](const unsigned int i) {
	return m_Mat[i];
}
//mat3x3
Mat3x3::Mat3x3() {
	m_Mat = glm::mat3x3(1);
}
glm::vec3 Mat3x3::operator*(const glm::vec3& v)const {
	return m_Mat * v;
}
glm::vec3& Mat3x3::operator[](const unsigned int i) {
	return m_Mat[i];
}