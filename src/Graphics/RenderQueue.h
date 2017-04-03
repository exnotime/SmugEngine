#pragma once

///This class is the interface between the graphics engine and the game engine
///Fetch the renderqueue for the current frame from the graphics engine 
#include "GraphicsObjects.h"
#include <vector>

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