#pragma once

///This class is the interface between the graphics engine and the game engine
///Fetch the renderqueue for the current frame from the graphics engine 
#include "GraphicsExport.h"
#include <glm/glm.hpp>
#include <vector>
struct CameraData {
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

class GFX_DLL RenderQueue {
public:
	RenderQueue();
	~RenderQueue();
	void Clear();

	void AddCamera(const CameraData& cd);
	std::vector<CameraData>& GetCameras();

	void AddModel();
private:
	std::vector<CameraData> m_Cameras;
};