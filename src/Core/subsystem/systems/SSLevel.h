#pragma once
#include "../SubSystem.h"
#include <AssetLoader/Resources.h>
#include <vector>

namespace smug {

	struct ProcVars {
		int width = 32;
		int height = 32;
		int depth = 32;
		int octaves = 6;
		float lacunarity = 2.0f;
		float gain = 0.5;
		float offset = 1.0f;
		float scale = 0.5;
	};

	class SSLevel : public SubSystem {
	public:
		SSLevel();
		~SSLevel();

		virtual void Startup();
		virtual void Update(const double deltaTime);
		virtual void Shutdown();
	private:
		void GenerateWorld(ProcVars vars);
		MeshInfo m_Mesh;
		ModelInfo m_WorldModel;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		MaterialInfo m_Material;
		ResourceHandle m_ModelHandle;
	};
}

