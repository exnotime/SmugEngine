#include "RenderQueue.h"

RenderQueue::RenderQueue(){
}

RenderQueue::~RenderQueue(){
}

void RenderQueue::Clear(){
	m_Cameras.clear();
}

void RenderQueue::AddCamera(const CameraData & cd){
	m_Cameras.push_back(cd);
}

std::vector<CameraData>& RenderQueue::GetCameras() {
	return m_Cameras;
}

void RenderQueue::AddModel(){

}
