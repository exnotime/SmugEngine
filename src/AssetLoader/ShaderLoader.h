#pragma once
#include <string>
#include "Resources.h"
class ShaderLoader {
  public:
	ShaderLoader();
	~ShaderLoader();
	char* LoadShaders(const std::string& filename, ShaderInfo& info);
};