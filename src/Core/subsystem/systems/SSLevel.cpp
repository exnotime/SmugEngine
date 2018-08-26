#include "SSLevel.h"
#include <Core/GlobalSystems.h>
#include <Core/components/TransformComponent.h>
#include <Core/components/ModelComponent.h>
#include <par_shapes.h>

#define STB_PERLIN_IMPLEMENTATION
#include <stb-master/stb_perlin.h>
#include <algorithm>
#include <dualmc/dualmc.h>

#include <Imgui/imgui.h>
using namespace smug;

SSLevel::SSLevel(){

}

SSLevel::~SSLevel(){

}

void SSLevel::Startup() {
	//par_shapes_mesh* cubeMesh;
	//cubeMesh = par_shapes_create_cube();
	////par_shapes_unweld(cubeMesh, true);
	//par_shapes_translate(cubeMesh, -0.5f, -0.5f, -0.5f);
	//par_shapes_unweld(cubeMesh, true);
	//par_shapes_compute_normals(cubeMesh);
	//std::vector<Vertex> verts;
	//for (uint32_t i = 0; i < cubeMesh->npoints; i++) {
	//	Vertex v;
	//	v.Position = glm::vec3(cubeMesh->points[i * 3 + 0], cubeMesh->points[i * 3 + 1], cubeMesh->points[i * 3 + 2]);
	//	glm::vec3 normal = glm::vec3(cubeMesh->normals[i * 3], cubeMesh->normals[i * 3 + 1], cubeMesh->normals[i * 3 + 2]);
	//	glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
	//	glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
	//	glm::vec3 tangent;
	//	if (glm::length(c1) > glm::length(c2)) {
	//		tangent = c1;
	//	}
	//	else {
	//		tangent = c2;
	//	}
	//	v.Normal = glm::vec3(glm::normalize(normal));
	//	v.Tangent = glm::vec3(glm::normalize(tangent));
	//	glm::vec2 uv;
	//	uv.x = glm::dot(glm::vec3(v.Normal), glm::vec3(1, 0, 0)) * 0.5f + 0.5f;
	//	uv.y = glm::dot(glm::vec3(v.Normal), glm::vec3(0, 1, 0)) * 0.5f + 0.5f;
	//	v.TexCoord = glm::vec2(uv);
	//	verts.push_back(v);
	//}
	//std::vector<uint32_t> indices;
	//for (uint32_t i = 0; i < cubeMesh->ntriangles; i++) {
	//	indices.push_back(cubeMesh->triangles[i * 3 + 0]);
	//	indices.push_back(cubeMesh->triangles[i * 3 + 1]);
	//	indices.push_back(cubeMesh->triangles[i * 3 + 2]);
	//}
	//Gen level
	ProcVars vars;
	vars.width = 32;
	vars.height = 32;
	vars.depth = 32;
	vars.lacunarity = 2.0f;
	vars.gain = 0.5f;
	vars.offset = 1.0f;
	vars.scale = 0.5f;
	vars.octaves = 6;
	GenerateWorld(vars);
	if (m_Indices.size() > 0) {
		m_ModelHandle = g_AssetLoader.LoadGeneratedModel(m_WorldModel, "Voxels");

		Entity& e = globals::g_EntityManager->CreateEntity();

		TransformComponent tc;
		tc.Position = glm::vec3(0, 1, 0);
		tc.Scale = glm::vec3(1.0f, 1.0f, 1.0f);
		globals::g_Components->CreateComponent(&tc, e, TransformComponent::Flag);

		ModelComponent mc;
		mc.ModelHandle = m_ModelHandle;
		mc.Static = false;
		mc.Tint = glm::vec4(0.4f, 0.5f, 0.4f, 1.0f);
		globals::g_Components->CreateComponent(&mc, e, ModelComponent::Flag);
	}
}

void SSLevel::Update(const double deltaTime) {
	ImGui::Begin("ProcGen");
	static ProcVars vars;
	ImGui::InputInt("Width", &vars.width);
	ImGui::InputInt("Height", &vars.height);
	ImGui::InputInt("Depth", &vars.depth);
	ImGui::DragFloat("Lacunarity", &vars.lacunarity,0.001, 0.0, 100.0f);
	ImGui::DragFloat("Gain", &vars.gain, 0.001, 0.0, 100.0f);
	ImGui::DragFloat("Offset", &vars.offset, 0.001, 0.0, 100.0f);
	ImGui::DragFloat("Scale", &vars.scale, 0.001, 0.0, 100.0f);
	ImGui::InputInt("Octaves", &vars.octaves);
	if (ImGui::Button("Rebuild voxel map")) {
		GenerateWorld(vars);
		if(m_Indices.size() > 0)
			g_AssetLoader.UpdateModel(m_ModelHandle, m_WorldModel);
	}
	ImGui::End();
}

void SSLevel::Shutdown() {

}

void SSLevel::GenerateWorld(ProcVars vars) {
	m_Vertices.resize(0);
	m_Indices.resize(0);

	const int size2 = vars.width * vars.height;
	uint8_t* voxelMap = (uint8_t*)malloc(vars.width * vars.height * vars.depth * sizeof(uint8_t));
	for (int z = 0; z < vars.depth; ++z) {
		for (int y = 0; y < vars.height; ++y) {
			for (int x = 0; x < vars.width; ++x) {
				glm::vec3 pos = glm::vec3(0, 1, 0) + glm::vec3(x, y, z) * glm::vec3(vars.scale);

				float p = stb_perlin_ridge_noise3(pos.x, pos.y, pos.z, vars.lacunarity, vars.gain, vars.offset, vars.octaves, vars.width, vars.height, vars.depth);
				//p = p + stb_perlin_fbm_noise3(x / (float)width, 1.0f, z / (float)depth, 2.0f, 0.5f, 6, width, height, depth);
				voxelMap[z * size2 + y * vars.height + x] = (p * 255U) > 128 ? 0 : 255;
			}
		}
	}
	dualmc::DualMC<uint8_t> dmc;
	std::vector<dualmc::Vertex> mcVertices;
	std::vector<dualmc::Quad> mcQuads;
	dmc.build(voxelMap, vars.width, vars.height, vars.depth, 128, false, false, mcVertices, mcQuads);

	for (auto& mv : mcVertices) {
		Vertex v;
		v.Position = glm::vec3(mv.x, mv.y, mv.z);
		v.Normal = glm::vec3(0, 1, 0);
		v.Tangent = glm::vec3(1, 0, 0);
		v.TexCoord = glm::vec2(0, 0);
		m_Vertices.push_back(v);
	}

	for (auto& q : mcQuads) {
		Vertex v0 = m_Vertices[q.i0];
		Vertex v1 = m_Vertices[q.i2];
		Vertex v2 = m_Vertices[q.i1];
		glm::vec3 normal = glm::normalize(glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
		if (!glm::any(glm::isinf(normal)) && !glm::any(glm::isnan(normal))) {
			m_Vertices[q.i0].Normal += normal;
			m_Vertices[q.i2].Normal += normal;
			m_Vertices[q.i1].Normal += normal;
		}
		v0 = m_Vertices[q.i0];
		v1 = m_Vertices[q.i3];
		v2 = m_Vertices[q.i2];
		normal = glm::normalize(glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
		if (!glm::any(glm::isinf(normal)) && !glm::any(glm::isnan(normal))) {
			m_Vertices[q.i0].Normal += normal;
			m_Vertices[q.i3].Normal += normal;
			m_Vertices[q.i2].Normal += normal;
		}
		m_Indices.push_back(q.i0);
		m_Indices.push_back(q.i1);
		m_Indices.push_back(q.i2);

		m_Indices.push_back(q.i0);
		m_Indices.push_back(q.i2);
		m_Indices.push_back(q.i3);
	}

	for (auto& v : m_Vertices) {
		glm::vec3& normal = v.Normal = -glm::normalize(v.Normal);
		glm::vec3 c1 = glm::cross(normal, glm::vec3(0.0, 0.0, 1.0));
		glm::vec3 c2 = glm::cross(normal, glm::vec3(0.0, 1.0, 0.0));
		glm::vec3 tangent;
		if (glm::length(c1) > glm::length(c2)) {
			tangent = c1;
		}
		else {
			tangent = c2;
		}
		v.Tangent = glm::normalize(tangent);
	}

	m_Mesh.IndexCount = m_Indices.size();
	m_Mesh.Indices = (uint32_t*)m_Indices.data();
	m_Mesh.Material = 0;
	m_Mesh.VertexCount = m_Vertices.size();
	m_Mesh.Vertices = (Vertex*)m_Vertices.data();

	m_WorldModel.MaterialCount = 0;
	m_WorldModel.MeshCount = 1;
	m_WorldModel.Materials = nullptr;
	m_WorldModel.Meshes = &m_Mesh;

	free(voxelMap);
}
