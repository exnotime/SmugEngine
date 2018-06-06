#pragma once
#include "GraphicsExport.h"
#include <glm/glm.hpp>
namespace smug {
struct GFX_DLL CameraData {
	glm::mat4 View = glm::mat4(1);
	glm::mat4 Proj = glm::mat4(1);
	glm::mat4 ProjView = glm::mat4(1);
	glm::vec3 Position = glm::vec3(0);
	glm::vec3 Forward = glm::vec3(0, 0, 1);
	glm::vec3 Right = glm::vec3(1, 0, 0);
	glm::vec3 Up = glm::vec3(0, -1, 0);
	float Fov = 0.61f;
	float Near = 0.1f;
	float Far = 100.0f;
	uint32_t Width = 16;
	uint32_t Height = 9;
};

struct GFX_DLL PointLight {
	glm::vec3 Color;
	float Range;
	glm::vec3 Position;
	float Intensity;
};

struct GFX_DLL SpotLight {
	glm::vec3 Color;
	float Range;
	glm::vec3 Position;
	float Intensity;
	glm::vec3 Direction;
	float Angle; //cos(angle)
};

struct GFX_DLL DirLight {
	glm::vec3 Color;
	float Intensity;
	glm::vec3 Direction;
	float padd;
};

struct SDFBox {
	glm::vec3 Bounds;
	float padd;
	glm::vec3 Pos;
	float padd2;
};

struct SDFSphere {
	glm::vec3 Pos;
	float Radius;
};

struct Cylinder {
	glm::vec3 Pos;
	float Radius;
};

}