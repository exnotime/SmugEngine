#pragma once
#include "../SubSystem.h"
#include <AssetLoader/Resources.h>
#include <Core/entity/Entity.h>
#include <Core/entity/EntityManager.h>
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
		uint8_t threshold = 128;
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
		MeshInfo m_VoxelMesh;
		ModelInfo m_VoxelModel;
		std::vector<Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		std::vector<Vertex> m_VoxelVertices;
		std::vector<uint32_t> m_VoxelIndices;
		MaterialInfo m_Material;
		ResourceHandle m_ModelHandle;
		ResourceHandle m_VoxelModelHandle;
		uint8_t* m_Voxels;
		uint32_t m_VoxelEntityUID;
	};
}

