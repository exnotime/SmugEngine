#pragma once
#include <AssetLoader/Resources.h>
namespace smug {
	struct ModelComponent {
		ResourceHandle ModelHandle;
		glm::vec4 Tint;
		bool Static = false;
		static unsigned int Flag;
	};
}

