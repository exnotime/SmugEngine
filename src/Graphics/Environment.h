#pragma once
#include "GraphicsObjects.h"
#include "Memory.h"

///this class manages the static lighting environment
class GFX_DLL Environment {
public:
	Environment();
	~Environment();
	void Init(const Memory* texMem, const std::string& iridianceProbe, const std::string& specProbe, const std::string& integratedIBL,
		uint32_t maxPointLights = 64, uint32_t maxDirLights = 4);

	void AddDirLight(const DirLight& dl);
	void AddPointLight(const PointLight& pl);

};