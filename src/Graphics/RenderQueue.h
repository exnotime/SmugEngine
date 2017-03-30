#pragma once

///This class is the interface between the graphics engine and the game engine
///Fetch the renderqueue for the current frame from the graphics engine 
#include "GraphicsExport.h"
#include <glm/glm.hpp>
#include <vector>
struct CameraData {
	glm::mat4 ViewProj;
	glm::vec3 Position;
};

class GFX_DLL RenderQueue {
public:
	RenderQueue();
	~RenderQueue();
	void Clear();

	void AddCamera(const CameraData& cd);
	void AddModel();
private:
	std::vector<CameraData> m_Cameras;
};