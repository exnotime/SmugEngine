#pragma once
#include <AssetLoader/Resources.h>
namespace smug {
struct ModelComponent {
	ResourceHandle ModelHandle;
	ResourceHandle Shader;
	glm::vec4 Tint;
	uint32_t Layer;
	bool Visible = true;
	bool Static = false;
	static unsigned int Flag;
};
}

