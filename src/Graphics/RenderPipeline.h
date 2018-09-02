#pragma once
#include "RenderInterface.h"
namespace smug {
	class RenderPipeline {
	public:
		RenderPipeline() {}
		~RenderPipeline() {}
		void InitFromScriptFile(const char* file);
	private:
	};
}